#include "ttc-discord/api.h"
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <openssl/ssl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <pthread.h>
#include <poll.h>
#include <pthread.h>


#include <discord.h>
#include <ttc-discord/messages.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-log.h>
#include <ttc-http.h>
#include <ttc-ws.h>

SSL_CTX *ssl_init() {
	SSL_library_init();

    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();

    return SSL_CTX_new(TLS_client_method());
}

SSL *ssl_socket_setup(SSL_CTX *ctx, int fd) {
	SSL *ssl;

	ssl = SSL_new(ctx);
	
	SSL_set_fd(ssl, fd);

	SSL_connect(ssl);

	return ssl;
}

int socket_create_from_host(const char *host, const char *port) {
	int sockfd, res;
	struct addrinfo *info;

	res = getaddrinfo(host, port, NULL, &info);
	if(res != 0) {
		TTC_LOG_WARN("getaddrinfo error %s\n", gai_strerror(res));
		return -1;
	}

	sockfd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
	if(sockfd < 0) {
		TTC_LOG_WARN("Socket error %s\n", strerror(errno));
		freeaddrinfo(info);
		return -1;	
	}

	res = connect(sockfd, info->ai_addr, (int)info->ai_addrlen);
	freeaddrinfo(info);
	if(res != 0) {
		TTC_LOG_WARN("Connect error %s", strerror(errno));
		close(sockfd);
		return -1;	
	}

	return sockfd;
}

void ttc_cmd_quit(ttc_discord_ctx_t *ctx) {
	ctx->running = 0;
}

int ttc_cmd_embed(ttc_discord_ctx_t *ctx) {
	ttc_discord_embed_t embed = {0};
	char *line = NULL, *tmp = NULL;
	size_t size = 0, total = 0;
	ssize_t length = 0;
	int res = 0;
	
	printf("Please Enter the embed's title: \n");
	length = getline(&line, &size, stdin);
	if(length < 0) { return -1; }
	embed.title = strndup(line, length);
	if(!embed.title) {
		goto title_error;
	}

	printf("Please Enter the description:(to end enter a line with just \"end\" written on it\n");

		length = getline(&line, &size, stdin);	
	do {
		if(length < 0) { goto description_realloc; }

		/*set tmp to new ptr and check it*/
		tmp = realloc(embed.description, length + total + 1); 
		if(!tmp) {
			goto description_realloc;
		}; /*This is unlikely to fail but check it*/
		
		embed.description = tmp;
		embed.description[total] = 0;
		strncat(embed.description, line, length);

		total += length;
		length = getline(&line, &size, stdin);
	} while(strcmp("end\n", line) != 0);

	printf("Channel ID to send message in:\n");
	length = getline(&line, &size, stdin);
	line[length - 1] = 0;

	printf("Sending Embed\n");
	ttc_discord_send_embed(&embed, ctx, strtoull(line, NULL, 0));

description_realloc:
	if(embed.title) {
		free(embed.title);
	}
	if(embed.description) {
		free(embed.description);
	}
title_error:
	if(line) {
		free(line);
	}

	return res;
}


void ttc_cmd_loop(ttc_discord_ctx_t *ctx) {
	char *line;
	size_t size;
	ssize_t length;
	
	line = NULL;
	size = 0;

	while(ctx->running) {
		printf("> ");
		fflush(stdout);
		length = getline(&line, &size, stdin);
		if(strncmp("embed\n", line, length) == 0) {
			ttc_cmd_embed(ctx);
		} else if(strncmp("quit\n", line, length) == 0) {
			ttc_cmd_quit(ctx);
		}		
	}
	
	/* Free the last line the rest where free'd by
	 * rentry of the getline function so we need to free
	 * the last one
	 */
	free(line); 
}

int ttc_discord_run(ttc_discord_ctx_t *ctx) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	json_object *json_res, *url;
	char *json_str;
	
	ctx->running = 1;

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, "/api/v10/gateway/bot");
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_GET);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_add_header(request, "Host", "discord.com");
	ttc_http_request_add_header(request, "Authorization", ctx->api_token);
	ttc_http_request_add_header(request, "User-Agent", "DiscordBot (https://github.com/CaitCatDev, 1)");
	ttc_http_request_build(request);

	ttc_https_request_send(request, ctx->api);

	response = ttc_https_get_response(ctx->api);
	if(response->status != 200) {
		return -1;
	}

	TTC_LOG_DEBUG(response->data);

	json_res = json_tokener_parse(response->data);
	url = json_object_object_get(json_res, "url");
	json_str = strdup(json_object_get_string(url));

	TTC_LOG_DEBUG("Gateway URL: %s\n", json_str);
	ctx->gateway_url = &json_str[6];
	ctx->gateway = ttc_wss_create_from_host(ctx->gateway_url, "443", ctx->ssl_ctx);
	
	ttc_http_request_free(request);
	ttc_http_response_free(response);
	json_object_put(json_res);
	free(json_str);

	discord_identify(ctx);
	pthread_create(&ctx->read_thread, NULL, discord_gateway_read, ctx);	

	ttc_cmd_loop(ctx);

	pthread_cancel(ctx->read_thread);
	pthread_cancel(ctx->heart_thread);

	pthread_join(ctx->read_thread, NULL);
	
	return 0;
}

void ttc_discord_ctx_destroy(ttc_discord_ctx_t *ctx) {
	cmd_listeners_t *tmp, *listener;
	
	free(ctx->command_callbacks);

	ttc_wss_free(ctx->gateway);

	free(ctx->api_token);

	SSL_shutdown(ctx->api);
	SSL_free(ctx->api);

	SSL_CTX_free(ctx->ssl_ctx);

	free(ctx);
}

int ttc_discord_parse_config(char *path, ttc_discord_ctx_t *ctx) {
	FILE *fp;
	char *line = NULL;
	size_t size = 0;

	fp = fopen(path, "r");
	
	while(getline(&line, &size, fp) != -1) {
		line[strcspn(line, "\n")] = 0; /*Remove newline*/
		if(strncmp(line, "TOKEN=", 5) == 0) {
			ctx->token = strdup(&line[6]);
		} else if(strncmp(line, "APP_ID=", 7) == 0) {
			ctx->app_id = strdup(&line[7]);
		} 
	}

	free(line);
	if(!ctx->token) {
		if(ctx->app_id) free(ctx->app_id);
		TTC_LOG_FATAL("Error token not in config file\n");
		return -1;
	} else if(!ctx->app_id) {
		free(ctx->token);
		TTC_LOG_FATAL("Error App id not in config\n");
		return -1;
	}

	return 0;
}


ttc_discord_ctx_t *ttc_discord_ctx_create(char *path) {
	ttc_discord_ctx_t *discord = calloc(1, sizeof(ttc_discord_ctx_t));
	int apisocket = 0, length;

	if(ttc_discord_parse_config(path, discord) < 0) {
		free(discord);
		return NULL;
	}

	TTC_LOG_INFO("Discord Token: %s\n", discord->token);
	TTC_LOG_INFO("Discord App Id: %s\n", discord->app_id);
	length = snprintf(NULL, 0, "Bot %s", discord->token);
	discord->api_token = calloc(1, length + 1);
	length = snprintf(discord->api_token, length + 1, "Bot %s", discord->token);
	
	discord->ssl_ctx = ssl_init();

	apisocket = socket_create_from_host("discord.com", "443");

	discord->api = ssl_socket_setup(discord->ssl_ctx, apisocket);	

	return discord;
}

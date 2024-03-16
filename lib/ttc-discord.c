#include "ttc-discord/api.h"
#include <errno.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <json-c/json_types.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <poll.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <ttc-http/sockets.h>
#include <unistd.h>

#include <discord.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-discord/messages.h>
#include <ttc-log.h>

SSL_CTX *ssl_init() {
	SSL_library_init();

	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	return SSL_CTX_new(TLS_client_method());
}

void ttc_discord_stop_bot(ttc_discord_ctx_t *ctx) {
	sem_post(&ctx->finish_sem);
}

int ttc_cmd_embed(ttc_discord_ctx_t *ctx) {
	ttc_discord_embed_t embed = {0};
	char *line = NULL, *tmp = NULL;
	size_t size = 0, total = 0;
	ssize_t length = 0;
	int res = 0;

	printf("Please Enter the embed's title: \n");
	length = getline(&line, &size, stdin);
	if (length < 0) {
		return -1;
	}
	embed.title = strndup(line, length);
	if (!embed.title) {
		goto title_error;
	}

	printf("Please Enter the description:(to end enter a line with just \"end\" written on it\n");

	length = getline(&line, &size, stdin);
	do {
		if (length < 0) {
			goto description_realloc;
		}

		/*set tmp to new ptr and check it*/
		tmp = realloc(embed.description, length + total + 1);
		if (!tmp) {
			goto description_realloc;
		}; /*This is unlikely to fail but check it*/

		embed.description = tmp;
		embed.description[total] = 0;
		strncat(embed.description, line, length);

		total += length;
		length = getline(&line, &size, stdin);
	} while (strcmp("end\n", line) != 0);

	printf("Channel ID to send message in:\n");
	length = getline(&line, &size, stdin);
	line[length - 1] = 0;

	printf("Sending Embed\n");
	ttc_discord_send_embed(&embed, ctx, strtoull(line, NULL, 0));

description_realloc:
	if (embed.title) {
		free(embed.title);
	}
	if (embed.description) {
		free(embed.description);
	}
title_error:
	if (line) {
		free(line);
	}

	return res;
}

static void ttc_cmd_loop_cleanup(void *vargp) {
	char **line_ptrptr = vargp;
	free(*line_ptrptr);
}

void *ttc_cli_thread(void *vargp) {
	ttc_discord_ctx_t *ctx = vargp;
	char *line;
	size_t size;
	ssize_t length;

	line = NULL;
	size = 0;

	pthread_cleanup_push(ttc_cmd_loop_cleanup, &line);

	while (1) {
		printf("> ");
		fflush(stdout);
		length = getline(&line, &size, stdin);
		if (strncmp("embed\n", line, length) == 0) {
			ttc_cmd_embed(ctx);
		} else if (strncmp("quit\n", line, length) == 0) {
			ttc_discord_stop_bot(ctx);
		}
	}
	pthread_cleanup_pop(1);
	pthread_exit(NULL);
}

int ttc_discord_run(ttc_discord_ctx_t *ctx) {
	sem_init(&ctx->finish_sem, 0, 0);
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	json_object *json_res, *url;
	char *json_str;

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, "/api/v10/gateway/bot");
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_GET);
	ttc_http_request_set_http_version(request, HTTP_VER_11);

	response = ttc_discord_api_send_request(ctx, request);

	TTC_LOG_DEBUG(response->data);

	json_res = json_tokener_parse(response->data);
	url = json_object_object_get(json_res, "url");
	json_str = strdup(json_object_get_string(url));

	TTC_LOG_DEBUG("Gateway URL: %s\n", json_str);
	ctx->gateway_url = &json_str[6];
	ctx->gateway = ttc_ws_create_from_host(ctx->gateway_url, "443", ctx->ssl_ctx);

	ttc_http_request_free(request);
	ttc_http_response_free(response);
	json_object_put(json_res);
	free(json_str);

	discord_identify(ctx);
	pthread_create(&ctx->read_thread, NULL, discord_gateway_read, ctx);
	pthread_create(&ctx->cli_thread, NULL, ttc_cli_thread, ctx);

	// when this semaphore gets available the bot stops
	sem_wait(&ctx->finish_sem);

	pthread_cancel(ctx->read_thread);
	pthread_join(ctx->read_thread, NULL);
	pthread_cancel(ctx->heart_thread);
	pthread_join(ctx->heart_thread, NULL);
	pthread_cancel(ctx->cli_thread);
	pthread_join(ctx->cli_thread, NULL);

	return 0;
}

void ttc_discord_ctx_destroy(ttc_discord_ctx_t *ctx) {
	free(ctx->command_callbacks);
	free(ctx->modal_callbacks);
	free(ctx->components_callbacks);

	ttc_ws_free(ctx->gateway);

	free(ctx->api_token);

	free(ctx->token);
	free(ctx->app_id);

	free(ctx->resume_url);
	free(ctx->session_id);

	ttc_http_socket_free(ctx->api);

	SSL_CTX_free(ctx->ssl_ctx);

	sem_destroy(&ctx->finish_sem);

	free(ctx);
}

int ttc_discord_parse_config(char *path, ttc_discord_ctx_t *ctx) {
	FILE *fp;
	char *line = NULL;
	size_t size = 0;

	fp = fopen(path, "r");

	if (!fp) {
		TTC_LOG_ERROR("Error opening config file: %s\n", strerror(errno));
		return -1;
	}

	while (getline(&line, &size, fp) != -1) {
		line[strcspn(line, "\n")] = 0; /*Remove newline*/
		if (strncmp(line, "TOKEN=", 5) == 0) {
			ctx->token = strdup(&line[6]);
		} else if (strncmp(line, "APP_ID=", 7) == 0) {
			ctx->app_id = strdup(&line[7]);
		}
	}

	free(line);
	if (!ctx->token) {
		free(ctx->app_id);
		TTC_LOG_FATAL("Error token not in config file\n");
		return -1;
	} else if (!ctx->app_id) {
		free(ctx->token);
		TTC_LOG_FATAL("Error App id not in config\n");
		return -1;
	}

	return 0;
}

ttc_discord_ctx_t *ttc_discord_ctx_create(char *path) {
	ttc_discord_ctx_t *discord = calloc(1, sizeof(ttc_discord_ctx_t));

	if (ttc_discord_parse_config(path, discord) < 0) {
		free(discord);
		return NULL;
	}

	TTC_LOG_INFO("Discord Token: %s\n", discord->token);
	TTC_LOG_INFO("Discord App Id: %s\n", discord->app_id);
	CREATE_SNPRINTF_STRING(discord->api_token, "Bot %s", discord->token);

	discord->ssl_ctx = ssl_init();

	discord->api = ttc_http_new_socket("discord.com", "443", discord->ssl_ctx);
	return discord;
}

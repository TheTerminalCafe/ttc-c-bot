#include <json-c/json.h>
#include <json-c/json_object.h>
#include <stdint.h>
#include <stdio.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/api.h>
#include <discord.h>
#include <ttc-log.h>
#include <ttc-http.h>

int ttc_discord_send_simple_message(const char *str, ttc_discord_ctx_t *ctx, uint64_t cid) {
	json_object *content, *message;
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *url;
	int length;
	const char *fmt = "/api/v10/channels/%lu/messages";
	
	content = json_object_new_string(str);
	message = json_object_new_object();

	
	json_object_object_add(message, "content", content);

	length = snprintf(NULL, 0, fmt, cid);

	url = calloc(1, length + 1);
	
	snprintf(url, length + 1, fmt, cid);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	
	response = ttc_discord_api_send_json(ctx, request, message);

	ttc_http_request_free(request);
	ttc_http_response_free(response);
	json_object_put(message);
	free(url);
	return 0;
}

int ttc_discord_send_embed(ttc_discord_embed_t *embed, ttc_discord_ctx_t *ctx, uint64_t cid) {
	json_object *json_embed, *embeds, *message;
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *url;
	int length;
	const char *fmt = "/api/v10/channels/%lu/messages";
	
	json_embed = ttc_discord_embed_to_json(embed);
	embeds = json_object_new_array();
	message = json_object_new_object();
	
	json_object_array_add(embeds, json_embed);
	json_object_object_add(message, "embeds", embeds);

	length = snprintf(NULL, 0, fmt, cid);

	url = calloc(1, length + 1);
	
	snprintf(url, length + 1, fmt, cid);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	
	response = ttc_discord_api_send_json(ctx, request, message);

	ttc_http_request_free(request);
	ttc_http_response_free(response);
	json_object_put(message);
	free(url);
	return 0;
}

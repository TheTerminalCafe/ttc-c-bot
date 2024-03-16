#include <discord.h>
#include <json-c/json.h>
#include <json-c/json_object.h>
#include <stdint.h>
#include <stdio.h>
#include <ttc-discord/api.h>
#include <ttc-discord/discord.h>

#include <ttc-log.h>

int ttc_discord_send_simple_message(const char *str, ttc_discord_ctx_t *ctx, uint64_t cid) {
	json_object *content, *message;
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *url;

	content = json_object_new_string(str);
	message = json_object_new_object();

	json_object_object_add(message, "content", content);

	CREATE_SNPRINTF_STRING(url, "/api/v10/channels/%" PRIu64 "/messages", cid);

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

int ttc_discord_edit_embed(ttc_discord_embed_t *embed, ttc_discord_ctx_t *ctx, uint64_t cid,
													 uint64_t mid) {
	json_object *json_embed, *embeds, *message;
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *url;

	printf("Message id: %" PRIu64 "\n", mid);

	json_embed = ttc_discord_embed_to_json(embed);
	embeds = json_object_new_array();
	message = json_object_new_object();

	CREATE_SNPRINTF_STRING(url, "/api/v10/channels/%" PRIu64 "/messages/%" PRIu64, cid, mid);

	json_object_array_add(embeds, json_embed);
	json_object_object_add(message, "embeds", embeds);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_add_header(request, "X-Ratelimit-Precision", "millisecond");
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_PATCH);

	response = ttc_discord_api_send_json(ctx, request, message);
	printf("%d\n", response->status);
	printf("response data %s\n", response->data);

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

	json_embed = ttc_discord_embed_to_json(embed);
	embeds = json_object_new_array();
	message = json_object_new_object();

	json_object_array_add(embeds, json_embed);
	json_object_object_add(message, "embeds", embeds);

	CREATE_SNPRINTF_STRING(url, "/api/v10/channels/%" PRIu64 "/messages", cid);

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

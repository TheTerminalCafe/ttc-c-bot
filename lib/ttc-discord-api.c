#include "ttc-discord/discord.h"
#include <discord.h>
#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ttc-discord/api.h>

#include <errno.h>

#include <ttc-log.h>

#include <ttc-http/request.h>
#include <ttc-http/response.h>
#include <ttc-http/sockets.h>

#include <json-c/json.h>

int ttc_discord_message_extract_embed(ttc_discord_ctx_t *ctx, snowflake_t cid, snowflake_t mid,
																			ttc_discord_embed_t *embed) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	json_object *message, *author, *embeds, *em;
	char *url = NULL;

	CREATE_SNPRINTF_STRING(url, "/api/v10/channels/%" PRIu64 "/messages/%" PRIu64, cid, mid);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_GET);

	free(url);

	response = ttc_discord_api_send_request(ctx, request);
	ttc_http_request_free(request);
	TTC_LOG_WARN("%s\n", response->data);

	if (response->status == 200) {
		message = json_tokener_parse(response->data);

		json_object_object_get_ex(message, "author", &author);
		if (strcmp(json_object_get_string(json_object_object_get(author, "id")), ctx->app_id) != 0) {
			TTC_LOG_ERROR("The is not a message by the bot can't decode this\n");
			json_object_put(message);
			ttc_http_response_free(response);
			return -1;
		}

		json_object_object_get_ex(message, "embeds", &embeds);
		em = json_object_array_get_idx(embeds, 0);

		embed->title = strdup(json_object_get_string(json_object_object_get(em, "title")));
		embed->description = strdup(json_object_get_string(json_object_object_get(em, "description")));
		json_object_put(message);
		ttc_http_response_free(response);
		return 0;
	}

	ttc_http_response_free(response);
	return 1;
}

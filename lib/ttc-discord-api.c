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
#include <ttc-http.h>
#include <ttc-log.h>
#include <ttc-ws.h>

#include <json-c/json.h>

int ttc_discord_message_extract_embed(ttc_discord_ctx_t *ctx, snowflake_t cid, snowflake_t mid,
																			ttc_discord_embed_t *embed) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	json_object *message, *author, *embeds, *field, *em;
	char *url = NULL;
	int length;

	length = snprintf(NULL, 0, "/api/v10/channels/%lu/messages/%lu", cid, mid);

	url = calloc(1, length + 1);

	length = snprintf(url, length + 1, "/api/v10/channels/%lu/messages/%lu", cid, mid);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_GET);

	response = ttc_discord_api_send_request(ctx, request);
	TTC_LOG_WARN("%s\n", response->data);
	printf("%s\n", ttc_http_request_get_str(request));

	length = response->status;
	if (length == 200) {

		message = json_tokener_parse(response->data);

		json_object_object_get_ex(message, "author", &author);
		if (strcmp(json_object_get_string(json_object_object_get(author, "id")), ctx->app_id) != 0) {
			TTC_LOG_ERROR("The is not a message by the bot can't decode this\n");
			return -1;
		}

		json_object_object_get_ex(message, "embeds", &embeds);
		em = json_object_array_get_idx(embeds, 0);

		embed->title = strdup(json_object_get_string(json_object_object_get(em, "title")));
		embed->description = strdup(json_object_get_string(json_object_object_get(em, "description")));
		return 0;
	}

	return 1;
}

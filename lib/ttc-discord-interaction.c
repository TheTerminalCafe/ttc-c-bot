#include <discord.h>
#include <stdint.h>
#include <ttc-discord/api.h>
#include <ttc-log.h>

bool ttc_discord_command_in_guild(uint64_t gid) {
	return gid != 0;
}

void ttc_discord_interaction_respond_embed(ttc_discord_ctx_t *ctx, char *title, char *description,
																					 uint32_t color, const char *url) {
	ttc_discord_embed_t embed = {0};
	json_object *embed_object, *type, *interaction, *data, *em_array;
	embed.title = title;
	embed.description = description;
	embed.color = color;

	interaction = json_object_new_object();
	data = json_object_new_object();
	type = json_object_new_int(DiscordInteractionCallbackMessage);
	embed_object = ttc_discord_embed_to_json(&embed);

	json_object_object_add(interaction, "type", type);
	em_array = json_object_new_array();
	json_object_array_add(em_array, embed_object);
	json_object_object_add(data, "embeds", em_array);
	json_object_object_add(interaction, "data", data);

	ttc_http_request_t *request = ttc_http_new_request();
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_path(request, url);

	ttc_http_response_t *response = ttc_discord_api_send_json(ctx, request, interaction);

	TTC_LOG_DEBUG("Response Embed returned: %d\n%s\n", response->status, response->headers);

	json_object_put(interaction);
	ttc_http_response_free(response);
	ttc_http_request_free(request);
}

void ttc_discord_interaction_loading(ttc_discord_ctx_t *ctx, const char *url) {
	ttc_discord_embed_t embed = {0};
	json_object *type, *interaction;

	interaction = json_object_new_object();
	type = json_object_new_int(DiscordInteractionCallbackDeferredMessage);

	json_object_object_add(interaction, "type", type);

	ttc_http_request_t *request = ttc_http_new_request();
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_path(request, url);

	ttc_http_response_t *response = ttc_discord_api_send_json(ctx, request, interaction);

	TTC_LOG_DEBUG("Response loading returned: %d\n%s\n", response->status, response->headers);

	json_object_put(interaction);
	ttc_http_response_free(response);
	ttc_http_request_free(request);
}

void ttc_discord_interaction_loading_respond(ttc_discord_ctx_t *ctx, char *title, char *description,
																						 uint32_t color,
																						 ttc_discord_interaction_t *interaction) {
	const char *fmt = "/api/v10/webhooks/%lu/%s/messages/@original";
	char *url;
	ttc_discord_embed_t embed = {0};
	json_object *embed_object, *message, *em_array;
	int length;

	length = snprintf(NULL, 0, fmt, interaction->app_id, interaction->token);
	url = calloc(1, length + 1);
	snprintf(url, length + 1, fmt, interaction->app_id, interaction->token);

	embed.title = title;
	embed.description = description;
	embed.color = color;

	message = json_object_new_object();
	embed_object = ttc_discord_embed_to_json(&embed);

	em_array = json_object_new_array();
	json_object_array_add(em_array, embed_object);
	json_object_object_add(message, "embeds", em_array);

	ttc_http_request_t *request = ttc_http_new_request();
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_PATCH);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_path(request, url);

	ttc_http_response_t *response = ttc_discord_api_send_json(ctx, request, message);
	TTC_LOG_DEBUG("Response update returned: %d\n%s\n", response->status, response->headers);

	free(url);
	json_object_put(message);
	ttc_http_response_free(response);
	ttc_http_request_free(request);
}

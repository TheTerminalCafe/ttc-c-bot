#include <ttc-discord/api.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-discord/messages.h>

void ttc_embed_modal_submit(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
														const char *url) {
	json_object *type, *content, *flags, *response, *data;
	char *length_str;
	ttc_discord_embed_t embed = {0};
	ttc_http_response_t *http_response;
	ttc_http_request_t *request;
	uint64_t channel_id = 0, message_id = 0;

	response = json_object_new_object();
	data = json_object_new_object();
	flags = json_object_new_int(1 << 6); /*EPHMERAL*/
	type = json_object_new_int(DiscordInteractionCallbackMessage);

	for (uint32_t i = 0; i < interaction->data.modal->field_count; i++) {
		if (strcmp(interaction->data.modal->fields[i].id, "embed_title") == 0) {
			embed.title = interaction->data.modal->fields[i].value;
		} else if (strcmp(interaction->data.modal->fields[i].id, "embed_desc") == 0) {
			embed.description = interaction->data.modal->fields[i].value;
		} else if (strcmp(interaction->data.modal->fields[i].id, "embed_channel") == 0) {
			channel_id = strtoull(interaction->data.modal->fields[i].value, NULL, 10);
		} else if (strcmp(interaction->data.modal->fields[i].id, "embed_color") == 0) {
			embed.color = strtoull(interaction->data.modal->fields[i].value, NULL, 16);
		} else if (strcmp(interaction->data.modal->fields[i].id, "old_message") == 0) {
			printf("Message id: %s\n", interaction->data.modal->fields[i].value);
			message_id = strtoull(interaction->data.modal->fields[i].value, NULL, 10);
		}
	}

	json_object_object_add(response, "type", type);
	json_object_object_add(response, "data", data);

	json_object_object_add(data, "flags", flags);

	if (channel_id == 0) {
		content = json_object_new_string("Channel ID has to be number!");

	} else if (message_id) {
		content = json_object_new_string("Your message is edited");
		ttc_discord_edit_embed(&embed, ctx, channel_id, message_id);
	} else {
		content = json_object_new_string("You embed is created!");
		ttc_discord_send_embed(&embed, ctx, channel_id);
	}
	json_object_object_add(data, "content", content);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);

	http_response = ttc_discord_api_send_json(ctx, request, response);

	ttc_http_request_free(request);
	ttc_http_response_free(http_response);
	json_object_put(response);
}

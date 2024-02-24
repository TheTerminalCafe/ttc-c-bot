#include "ttc-discord/api.h"
#include <discord.h>
#include <json-c/json_object.h>

#include <ttc-http/request.h>
#include <ttc-http/response.h>
#include <ttc-log.h>

int ttc_discord_send_embed_modal(ttc_discord_ctx_t *ctx, const char *url) {
	json_object *id, *title, *components, *row, *submit, *title_input, *sysinfo, *desc_lable,
			*row_type, *title_type, *title_style, *sysinfo_type, *sysinfo_style, *desc_id, *desc_type,
			*desc_style, *title_id, *label, *modal_id, *description, *catergory, *modal, *row_components;

	modal = json_object_new_object();
	modal_id = json_object_new_string("ticket_modal");
	json_object_object_add(modal, "custom_id", modal_id);
	components = json_object_new_array();
	json_object_object_add(modal, "components", components);

	row_components = json_object_new_array();
	id = json_object_new_string("embed_submit");
	row = json_object_new_object();
	row_type = json_object_new_int(DiscordComponentActionRow);

	json_object_object_add(row, "type", row_type);
	json_object_object_add(row, "components", row_components);
	json_object_array_add(components, row);

	title_input = json_object_new_object();
	title = json_object_new_string("Create Embed:");
	json_object_object_add(modal, "title", title);
	label = json_object_new_string("Embed Title:");
	title_type = json_object_new_int(DiscordComponentTextInput);
	title_style = json_object_new_int(DiscordTextInputSingleLine);

	desc_lable = json_object_new_string("Embed Description:");
	desc_type = json_object_new_int(DiscordComponentTextInput);
	desc_style = json_object_new_int(DiscordTextInputParagraph);
	description = json_object_new_object();

	title_id = json_object_new_string("embed_title");
	desc_id = json_object_new_string("embed_desc");

	json_object_object_add(title_input, "type", title_type);
	json_object_object_add(title_input, "style", title_style);
	json_object_object_add(title_input, "label", label);
	json_object_object_add(title_input, "custom_id", title_id);

	json_object_object_add(description, "type", desc_type);
	json_object_object_add(description, "style", desc_style);
	json_object_object_add(description, "label", desc_lable);
	json_object_object_add(description, "custom_id", desc_id);

	json_object_array_add(components, title_input);
	json_object_array_add(components, description);

	ttc_http_request_t *request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	ttc_http_request_set_http_version(request, HTTP_VER_11);

	TTC_LOG_DEBUG("URL: %s\n", url);
	ttc_discord_api_send_json(ctx, request, modal);

	return 0;
}

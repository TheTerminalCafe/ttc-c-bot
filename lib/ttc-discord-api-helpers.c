#include "ttc-discord/discord.h"
#include <json-c/json.h>

#include <discord.h>
#include <errno.h>
#include <json-c/json_object.h>
#include <stddef.h>
#include <ttc-discord/api.h>
#include <ttc-log.h>

json_object *ttc_discord_embed_to_json(ttc_discord_embed_t *embed) {
	json_object *json_embed, *name, *desc, *color, *required;
	int result;

	json_embed = json_object_new_object();

	name = json_object_new_string(embed->title);
	desc = json_object_new_string(embed->description);
	color = json_object_new_int(embed->color);

	/* So we can essentially run this again and again
	 * and just check for failure
	 */
	json_object_object_add(json_embed, "title", name);
	json_object_object_add(json_embed, "description", desc);
	json_object_object_add(json_embed, "color", color);

	return json_embed;
}

json_object *ttc_discord_form_to_json(ttc_discord_modal_t *modal) {
	json_object *object, *title, *id, *fields[5], *field_ids[5], *field_labels[5], *field_style[5],
			*field_types[5], *action_row[5], *ar_type[5], *components, *ar_components[5], *required[5],
			*field_values[5];

	for (size_t ind = 0; ind < modal->field_count; ind++) {
		/*This function is intended to construct
		 * text input forms and not other modals*/
		if (modal->fields[ind].type != DiscordComponentTextInput) {
			return NULL;
		}
	}

	object = json_object_new_object();

	title = json_object_new_string(modal->name);
	id = json_object_new_string(modal->id);
	components = json_object_new_array();

	for (size_t ind = 0; ind < modal->field_count; ind++) {

		action_row[ind] = json_object_new_object();
		ar_type[ind] = json_object_new_int(DiscordComponentActionRow);
		ar_components[ind] = json_object_new_array();

		fields[ind] = json_object_new_object();
		field_ids[ind] = json_object_new_string(modal->fields[ind].id);
		field_style[ind] = json_object_new_int(modal->fields[ind].style);
		field_types[ind] = json_object_new_int(modal->fields[ind].type);
		field_labels[ind] = json_object_new_string(modal->fields[ind].label);
		required[ind] = json_object_new_boolean(modal->fields[ind].required);
		field_values[ind] = json_object_new_string(modal->fields[ind].value);

		json_object_object_add(fields[ind], "label", field_labels[ind]);
		json_object_object_add(fields[ind], "style", field_style[ind]);
		json_object_object_add(fields[ind], "type", field_types[ind]);
		json_object_object_add(fields[ind], "custom_id", field_ids[ind]);
		json_object_object_add(fields[ind], "required", required[ind]);
		json_object_object_add(fields[ind], "value", field_values[ind]);

		json_object_array_add(ar_components[ind], fields[ind]);

		json_object_object_add(action_row[ind], "type", ar_type[ind]);
		json_object_object_add(action_row[ind], "components", ar_components[ind]);

		json_object_array_add(components, action_row[ind]);
	}

	json_object_object_add(object, "custom_id", id);
	json_object_object_add(object, "title", title);
	json_object_object_add(object, "components", components);

	return object;
}

/*Wrap the headers discord always needs in this function so we don't
 * have to retype them for each reqeust we make and can instead just call
 * this function
 *
 * returns negative for error or positive HTTP status code
 */
ttc_http_response_t *ttc_discord_api_send_request(ttc_discord_ctx_t *ctx,
																									ttc_http_request_t *request) {
	ttc_http_request_add_header(request, "Host", "discord.com");
	ttc_http_request_add_header(request, "Authorization", ctx->api_token);
	ttc_http_request_add_header(request, "User-Agent",
															"DiscordBot (https://github.com/CaitCatDev, 1.0)");

	ttc_http_request_build(request);

	ttc_https_request_send(request, ctx->api);

	return ttc_https_get_response(ctx->api);
}

ttc_http_response_t *ttc_discord_api_send_json(ttc_discord_ctx_t *ctx, ttc_http_request_t *request,
																							 json_object *message) {
	char *length_str;
	int length;

	ttc_http_request_add_header(request, "Content-Type", "application/json");

	length = snprintf(NULL, 0, "%lu", strlen(json_object_to_json_string(message)));
	length_str = calloc(1, length + 1);
	snprintf(length_str, length + 1, "%lu", strlen(json_object_to_json_string(message)));
	ttc_http_request_add_header(request, "Content-Length", length_str);

	ttc_http_request_add_data(request, json_object_to_json_string(message));

	if (message) {
		free(length_str);
	}
	return ttc_discord_api_send_request(ctx, request);
}

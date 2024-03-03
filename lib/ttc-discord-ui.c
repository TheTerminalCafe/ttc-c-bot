#include <discord.h>
#include <json-c/json_object.h>
#include <stdint.h>
#include <ttc-discord/api.h>
#include <ttc-discord/discord.h>

#include <ttc-log.h>

int ttc_discord_add_component_listener(ttc_discord_ctx_t *ctx, const char *modal_id,
																			 void (*callback)(ttc_discord_interaction_t *,
																												ttc_discord_ctx_t *, const char *)) {
	cmd_listeners_t listener, *tmp;
	tmp = realloc(ctx->components_callbacks, (ctx->components + 1) * sizeof(cmd_listeners_t));

	listener.cmd_callback = callback;
	listener.name = modal_id;

	tmp[ctx->components] = listener;

	ctx->components_callbacks = tmp;
	ctx->components++;

	return 0;
}

int ttc_discord_create_button(ttc_discord_ctx_t *ctx, const char *btn_id, int btn_style,
															const char *text, uint64_t channel) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *length_str, *url;
	int result;
	json_object *message, *components, *button, *type, *label, *style, *id, *row, *artype,
			*arcomponents;

	/*Create an action row for the button*/
	artype = json_object_new_int(DiscordComponentActionRow);
	row = json_object_new_object();
	arcomponents = json_object_new_array();

	json_object_object_add(row, "type", artype);
	json_object_object_add(row, "components", arcomponents);

	/*Create the button*/
	components = json_object_new_array();
	message = json_object_new_object();
	button = json_object_new_object();
	type = json_object_new_int(DiscordComponentButton);
	label = json_object_new_string(text);
	id = json_object_new_string(btn_id);
	style = json_object_new_int(btn_style);

	json_object_object_add(button, "type", type);
	json_object_object_add(button, "custom_id", id);
	json_object_object_add(button, "label", label);
	json_object_object_add(button, "style", style);
	json_object_array_add(components, row);
	json_object_array_add(arcomponents, button);
	json_object_object_add(message, "components", components);

	CREATE_SNPRINTF_STRING(length_str, "%zu", strlen(json_object_to_json_string(message)));
	CREATE_SNPRINTF_STRING(url, "/api/v10/channels/%" PRIu64 "/messages", channel);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);

	response = ttc_discord_api_send_json(ctx, request, message);
	result = response->status;

	free(url);
	free(length_str);
	ttc_http_response_free(response);
	ttc_http_request_free(request);
	json_object_put(message);
	return result;
}

int ttc_discord_create_select_menu(ttc_discord_ctx_t *ctx, uint32_t type, const char *menu_id,
																	 uint64_t channel, uint32_t max) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *length_str, *url;
	int result;
	json_object *message, *components, *menu, *menu_type, *label, *style, *id, *row, *artype,
			*arcomponents, *max_val;
	/*Create an action row for the button*/
	artype = json_object_new_int(DiscordComponentActionRow);
	row = json_object_new_object();
	arcomponents = json_object_new_array();

	json_object_object_add(row, "type", artype);
	json_object_object_add(row, "components", arcomponents);

	/*Create the button*/
	components = json_object_new_array();
	message = json_object_new_object();
	menu = json_object_new_object();
	menu_type = json_object_new_int(type);
	id = json_object_new_string(menu_id);
	max_val = json_object_new_int(max);

	json_object_object_add(menu, "type", menu_type);
	json_object_object_add(menu, "custom_id", id);
	json_object_array_add(components, row);
	json_object_array_add(arcomponents, menu);
	json_object_object_add(message, "components", components);
	json_object_object_add(menu, "max_values", max_val);

	CREATE_SNPRINTF_STRING(length_str, "%zu", strlen(json_object_to_json_string(message)));
	CREATE_SNPRINTF_STRING(url, "/api/v10/channels/%" PRIu64 "/messages", channel);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);

	response = ttc_discord_api_send_json(ctx, request, message);
	result = response->status;

	free(url);
	free(length_str);
	ttc_http_response_free(response);
	ttc_http_request_free(request);
	json_object_put(message);
	return result;
}

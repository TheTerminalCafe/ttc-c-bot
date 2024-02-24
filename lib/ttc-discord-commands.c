#include "ttc-discord/api.h"
#include <json-c/json_object.h>
#include <json-c/json_types.h>

#include <discord.h>
#include <stdlib.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-log.h>

#include <ttc-http/request.h>
#include <ttc-http/response.h>

int discord_app_register_command_listener(ttc_discord_ctx_t *ctx, const char *title,
																					void (*callback)(ttc_discord_interaction_t *interaction,
																													 ttc_discord_ctx_t *ctx,
																													 const char *url)) {
	cmd_listeners_t listener, *tmp;

	tmp = realloc(ctx->command_callbacks, (ctx->callbacks + 1) * sizeof(cmd_listeners_t));

	listener.cmd_callback = callback;
	listener.name = title;

	tmp[ctx->callbacks] = listener;

	ctx->command_callbacks = tmp;
	ctx->callbacks++;

	return 0;
}

int discord_create_application_command(command_t *command, ttc_discord_ctx_t *ctx,
																			 void (*callback)(ttc_discord_interaction_t *interaction,
																												ttc_discord_ctx_t *ctx, const char *url)) {
	json_object *command_json, *name, *type, *description, *options, *allow_in_dms,
			*default_permissions;
	json_object *option_names[25], *option_desc[25], *option_type[25], *required[25],
			*option_objs[25];
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *url, *permissions;
	int result, length;

	length = snprintf(NULL, 0, "%lu", command->default_permissions);

	permissions = calloc(1, length + 1);
	if (!permissions) {
		TTC_LOG_DEBUG("Allocating Length string failed\n");
		return -1;
	}

	snprintf(permissions, length + 1, "%lu", command->default_permissions);

	command_json = json_object_new_object();
	name = json_object_new_string(command->name);
	description = json_object_new_string(command->description);
	type = json_object_new_int(command->type);
	allow_in_dms = json_object_new_boolean(command->allow_in_dms);
	default_permissions = json_object_new_string(permissions);
	options = json_object_new_array();

	/*construct the options from the struct*/
	for (int i = 0; i < command->option_count; i++) {
		if (i >= 25) {
			TTC_LOG_WARN("Too many options passed with command %s. 25 is the maximum number of options\n",
									 command->name);
			return -1;
		}
		option_objs[i] = json_object_new_object();
		required[i] = json_object_new_boolean(command->options[i].required);
		option_desc[i] = json_object_new_string(command->options[i].description);
		option_names[i] = json_object_new_string(command->options[i].name);
		option_type[i] = json_object_new_int(command->options[i].type);
		json_object_object_add(option_objs[i], "required", required[i]);
		json_object_object_add(option_objs[i], "description", option_desc[i]);
		json_object_object_add(option_objs[i], "type", option_type[i]);
		json_object_object_add(option_objs[i], "name", option_names[i]);
		json_object_array_add(options, option_objs[i]);
	}

	json_object_object_add(command_json, "name", name);
	json_object_object_add(command_json, "type", type);
	json_object_object_add(command_json, "description", description);
	json_object_object_add(command_json, "options", options);
	json_object_object_add(command_json, "dm_permission", allow_in_dms);
	json_object_object_add(command_json, "default_member_permissions", default_permissions);

	length = snprintf(NULL, 0, "/api/v10/applications/%s/commands", ctx->app_id);

	url = calloc(1, length + 1);
	if (!url) {
		TTC_LOG_DEBUG("Allocating Length string failed\n");
		free(permissions);
		return -1;
	}

	length = snprintf(url, length + 1, "/api/v10/applications/%s/commands", ctx->app_id);

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);

	response = ttc_discord_api_send_json(ctx, request, command_json);

	result = response->status;

	discord_app_register_command_listener(ctx, command->name, callback);

	ttc_http_response_free(response);
	ttc_http_request_free(request);
	free(permissions);
	free(url);
	json_object_put(command_json);
	return 0;
}

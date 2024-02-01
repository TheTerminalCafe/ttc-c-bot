#include <json-c/json_object.h>
#include <json-c/json_types.h>

#include <discord.h>
#include <stdlib.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-http.h>
#include <ttc-log.h>

int discord_app_register_command_listener(ttc_discord_ctx_t *ctx, const char *title,
			void (*callback)(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url)) {
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
		void (*callback)(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url)) {
	json_object *command_json, *name, *type, *description, *options, *allow_in_dms;
	json_object *option_names[25], *option_desc[25], *option_type[25], *required[25],
				*option_objs[25];
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	char *length_str, *url; 
	int result;

	command_json = json_object_new_object();
	name = json_object_new_string(command->name);
	description = json_object_new_string(command->description);
	type = json_object_new_int(command->type);
	allow_in_dms = json_object_new_boolean(command->allow_in_dms);
	options = json_object_new_array();
	
	/*construct the options from the struct*/
	for(int i = 0; i < command->option_count; i++) {
		if(i >= 25) {
			TTC_LOG_WARN("Too many options passed with command %s. 25 is the maximum number of options\n", command->name);
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

	int length = snprintf(NULL, 0, "%lu", strlen(json_object_to_json_string(command_json)));

	length_str = calloc(1, length + 1);
	if(!length_str) {
		TTC_LOG_DEBUG("Allocating Length string failed\n");
		return -1;
	}

	snprintf(length_str, length + 1, "%lu", strlen(json_object_to_json_string(command_json)));

	length = snprintf(NULL, 0, "/api/v10/applications/%s/commands", ctx->app_id);

	url = calloc(1, length + 1);
	if(!length_str) {
		TTC_LOG_DEBUG("Allocating Length string failed\n");
		return -1;
	}

	length = snprintf(url, length + 1, "/api/v10/applications/%s/commands", ctx->app_id);



	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);

	ttc_http_request_add_header(request, "Host", "discord.com");
	ttc_http_request_add_header(request, "Authorization", ctx->api_token);
	ttc_http_request_add_header(request, "Content-Type", "application/json");
	ttc_http_request_add_header(request, "Content-Length", length_str);
	ttc_http_request_add_header(request, "User-Agent", "DiscordBot (https://github.com/CaitCatDev, 1)");
	ttc_http_request_add_data(request, json_object_to_json_string(command_json));
	ttc_http_request_build(request);

	ttc_https_request_send(request, ctx->api);

	printf("%s\n", ttc_http_request_get_str(request));


	response = ttc_https_get_response(ctx->api);

	result = response->status;
	printf("%d\n%s\n", response->status, response->data);

	discord_app_register_command_listener(ctx, command->name, callback);

	ttc_http_response_free(response);
	ttc_http_request_free(request);
	free(url);
	free(length_str);
	json_object_put(command_json);
	return 0;
}

#include <json-c/json.h>
#include <json-c/json_object.h>
#include <json-c/json_types.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ttc-log.h>
#include <unistd.h>

#include <ttc-http/request.h>
#include <ttc-http/response.h>

#include <discord.h>
#include <ttc-discord/api.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-discord/messages.h>

void discord_identify(ttc_discord_ctx_t *ctx) {
	json_object *login, *properties, *os, *browser, *device, *intents, *opcode, *token, *data;
	ttc_ws_wrreq_t login_request;

	login = json_object_new_object();
	data = json_object_new_object();
	opcode = json_object_new_int(DiscordIdentify);

	/*properties setup*/
	properties = json_object_new_object();
	os = json_object_new_string("Linux");
	browser = json_object_new_string("ttc-cbot");
	device = json_object_new_string("ttc-cbot");
	token = json_object_new_string(ctx->token);

	intents = json_object_new_int((1 << 9) | (1 << 4) | (1 << 10));

	/*Fill Json Objects*/
	json_object_object_add(properties, "os", os);
	json_object_object_add(properties, "browser", browser);
	json_object_object_add(properties, "device", device);

	json_object_object_add(data, "token", token);
	json_object_object_add(data, "properties", properties);
	json_object_object_add(data, "intents", intents);

	json_object_object_add(login, "op", opcode);
	json_object_object_add(login, "d", data);

	login_request.res = 0;
	login_request.opcode = 1;
	login_request.fin = 1;
	login_request.mask = 1;
	login_request.data = strdup(json_object_to_json_string(login));
	login_request.len = strlen(json_object_to_json_string(login));

	ttc_ws_write(ctx->gateway, login_request);

	free(login_request.data);
	json_object_put(login);
}

void discord_heartbeat(ttc_discord_ctx_t *ctx) {
	static json_object *op, *d, *heartbeat;
	static ttc_ws_wrreq_t heartreq;

	heartbeat = json_object_new_object();
	d = json_object_new_int(ctx->sequence);
	op = json_object_new_int(1);
	json_object_object_add(heartbeat, "op", op);
	json_object_object_add(heartbeat, "d", d);

	heartreq.mask = 1;
	heartreq.res = 0;
	heartreq.fin = 1;
	heartreq.opcode = 1;
	heartreq.data = strdup(json_object_get_string(heartbeat));
	heartreq.len = strlen(heartreq.data);

	ttc_ws_write(ctx->gateway, heartreq);
	free(heartreq.data);
	json_object_put(heartbeat);
}

static void *discord_heart(void *vargp) {
	ttc_discord_ctx_t *ctx = vargp;
	struct timespec tmspec;

	tmspec.tv_sec = ctx->heart_interval / 1000;
	tmspec.tv_sec -= 3;
	tmspec.tv_nsec = (ctx->heart_interval - tmspec.tv_sec * 1000) * 1000000;

	while (1) {
		// clang-format off
		while(nanosleep(&tmspec, &tmspec));
		// clang-format on
		discord_heartbeat(ctx);
	}
}

void discord_reconnect(ttc_discord_ctx_t *ctx) {
	json_object *resume, *op, *seq, *d, *sessionid, *token;
	ttc_ws_wrreq_t wreq;

	pthread_cancel(ctx->heart_thread);

	resume = json_object_new_object();
	sessionid = json_object_new_string(ctx->session_id);
	token = json_object_new_string(ctx->token);
	op = json_object_new_int(6);
	seq = json_object_new_int(ctx->sequence);
	d = json_object_new_object();

	json_object_object_add(d, "token", token);
	json_object_object_add(d, "session_id", sessionid);
	json_object_object_add(d, "seq", seq);
	json_object_object_add(resume, "op", op);
	json_object_object_add(resume, "d", d);

	ttc_ws_free(ctx->gateway);

	ttc_ws_t *ws = ttc_ws_create_from_host(ctx->resume_url, "443", ctx->ssl_ctx);

	wreq.data = strdup(json_object_to_json_string(resume));
	wreq.len = strlen(json_object_to_json_string(resume));
	wreq.fin = 1;
	wreq.mask = 1;
	wreq.res = 0;
	wreq.opcode = TTC_WS_TEXT_FRAME;

	TTC_LOG_DEBUG("Resuming: %s\n", wreq.data);

	ttc_ws_write(ws, wreq);

	json_object_put(resume);
	free(wreq.data);
	ctx->gateway = ws;
}

void handle_interaction_app_command(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
																		const char *url) {
	uint64_t size;
	TTC_LOG_DEBUG("App Command\n");

	if (!ctx->command_callbacks) {
		TTC_LOG_ERROR("There is no command listeners registered?\n"
									"Did you manually make this command?\n");
		return;
	}

	for (size = 0; size < ctx->callbacks; size++) {
		TTC_LOG_DEBUG("Command: %s\n", ctx->command_callbacks[size].name);
		if (strcmp(ctx->command_callbacks[size].name, interaction->data.command->name) == 0) {
			ctx->command_callbacks[size].cmd_callback(interaction, ctx, url);
			return;
		}
	}
}

void handle_interaction_msg_component(ttc_discord_interaction_t *interaction,
																			ttc_discord_ctx_t *ctx, const char *url) {
	uint64_t size;
	TTC_LOG_DEBUG("Component Submitted: %s\n", interaction->data.component->id);

	for (size = 0; size < ctx->components; size++) {
		TTC_LOG_DEBUG("Component: %s\n", ctx->components_callbacks[size].name);
		if (strcmp(ctx->components_callbacks[size].name, interaction->data.component->id) == 0) {
			ctx->components_callbacks[size].cmd_callback(interaction, ctx, url);
			return;
		}
	}
}

void handle_interaction_modal_submit(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
																		 const char *url) {
	uint64_t size;
	TTC_LOG_DEBUG("Modal Submitted: %s\n", interaction->data.modal->id);

	for (size = 0; size < ctx->modals; size++) {
		TTC_LOG_DEBUG("Modal: %s\n", ctx->modal_callbacks[size].name);
		if (strcmp(ctx->modal_callbacks[size].name, interaction->data.modal->id) == 0) {
			ctx->modal_callbacks[size].cmd_callback(interaction, ctx, url);
			return;
		}
	}
}

static ttc_discord_app_cmd_opt_t ttc_dc_command_resolve_option(json_object *data) {
	ttc_discord_app_cmd_opt_t option = {0};
	json_object *object;

	json_object_object_get_ex(data, "type", &object);
	option.type = json_object_get_int64(object);
	TTC_LOG_DEBUG("Type: %lu\n", option.type);

	json_object_object_get_ex(data, "name", &object);

	/*Only copy upto DISCORD_OPTION_NAME_LENGTH to buffer
	 * STRNCPY shouldn't strictly be needed as discord says its
	 * Less than or equal to that size but we just choose to be extra
	 * safe in case their is ever some mistake on their end
	 */
	strncpy(option.name, json_object_get_string(object), DISCORD_OPTION_NAME_LENGTH);
	TTC_LOG_DEBUG("Name: %s\n", option.name);

	json_object_object_get_ex(data, "value", &object);
	switch (option.type) {
		case DiscordOptionRole:
		case DiscordOptionUser:
		case DiscordOptionChannel:
		case DiscordOptionString:
			/**Duplicate the string to mutable one. But also the current string is built into
			 * the json object and it's memory so we don't want to free that until
			 * we are also done with the string but we can just dup it so we
			 * can now free the json whenever we want.
			 *
			 * Just remember to free this with the object
			 */
			option.value.string = strdup(json_object_get_string(object));
			break;
		case DiscordOptionInteger:
			option.value.integer = json_object_get_int64(object);
			break;
		case DiscordOptionDouble:
			option.value.floating = json_object_get_double(object);
			break;
		case DiscordOptionBool:
			option.value.floating = json_object_get_boolean(object);
			break;
		/*TODO: Mentionable and attachments*/
		default:
			TTC_LOG_WARN("Unknown Option type of %d\n", option.type);
	}

	/**TODO:*/
	option.focused = false;

	/**We don't use this but TODO:*/
	option.options = NULL;
	return option;
}

static ttc_discord_app_cmd_data_t *ttc_discord_interaction_resolve_app_cmd_data(json_object *data) {
	json_object *object;
	size_t size, ind;
	json_bool found;
	ttc_discord_app_cmd_data_t *output;

	output = calloc(1, sizeof(ttc_discord_app_cmd_data_t));

	json_object_object_get_ex(data, "type", &object);

	output->type = json_object_get_int64(object);

	json_object_object_get_ex(data, "name", &object);
	strncpy(output->name, json_object_get_string(object), DISCORD_COMMAND_NAME_LENGTH);

	json_object_object_get_ex(data, "id", &object);
	output->id = strtoull(json_object_get_string(object), NULL, 10);

	found = json_object_object_get_ex(data, "guild_id", &object);
	output->guildid = found ? strtoull(json_object_get_string(object), NULL, 10) : 0;

	found = json_object_object_get_ex(data, "target_id", &object);
	output->targetid = found ? strtoull(json_object_get_string(object), NULL, 10) : 0;

	found = json_object_object_get_ex(data, "options", &object);
	if (found) {
		size = json_object_array_length(object);
		output->options = calloc(size, sizeof(ttc_discord_app_cmd_opt_t));
		output->opt_count = size;
		for (ind = 0; ind < size; ++ind) {
			output->options[ind] = ttc_dc_command_resolve_option(json_object_array_get_idx(object, ind));
		}
	}

	/* TODO: Resolved Data */
	return output;
}

ttc_discord_component_data_t *ttc_discord_interaction_resolve_component(json_object *data) {
	ttc_discord_component_data_t *component;
	json_object *array, *object;
	size_t index = 0;

	component = calloc(1, sizeof(ttc_discord_component_data_t));

	json_object_object_get_ex(data, "custom_id", &object);
	component->id = strdup(json_object_get_string(object));

	json_object_object_get_ex(data, "component_type", &object);
	component->type = json_object_get_uint64(object);

	/*It doesn't seem to be possible for ActionRow or TextInput to
	 * actually happen but I just have them here as a "better safe than sorry"
	 * mentalitty
	 */
	if (component->type != DiscordComponentButton || component->type != DiscordComponentTextInput ||
			component->type != DiscordComponentActionRow) {

		json_object_object_get_ex(data, "values", &array);
		component->count = json_object_array_length(array);

		/*allocate pointers for all the string dups*/
		component->values = calloc(component->count, sizeof(char *));

		for (index = 0; index < component->count; index++) {
			object = json_object_array_get_idx(array, index);
			component->values[index] = strdup(json_object_get_string(object));
		}
	}

	return component;
}

ttc_discord_modal_t *ttc_discord_interaction_resolve_modal(json_object *data) {
	json_object *id, *components, *row_components, *component, *rows;
	ttc_discord_modal_t *modal = calloc(1, sizeof(ttc_discord_modal_t));

	json_object_object_get_ex(data, "custom_id", &id);
	json_object_object_get_ex(data, "components", &components);

	modal->id = strdup(json_object_get_string(id));

	for (size_t ind = 0; ind < json_object_array_length(components); ind++) {
		rows = json_object_array_get_idx(components, ind);
		json_object_object_get_ex(rows, "components", &row_components);
		component = json_object_array_get_idx(row_components, 0);
		/*TODO: Error checking allocations*/
		/*we kinda just assume they are all text fields which is by no means a gurantee*/
		modal->fields[ind].id =
				strdup(json_object_get_string(json_object_object_get(component, "custom_id")));
		modal->fields[ind].value =
				strdup(json_object_get_string(json_object_object_get(component, "value")));
	}

	modal->field_count = json_object_array_length(components);

	return modal;
}

void ttc_discord_member_free(ttc_discord_member_t *member) {
	if (member->role_count) {
		for (size_t index = 0; index < member->role_count; index++) {
			free(member->roles[index]);
		}
		free(member->roles);
	}

	free(member);
}

void ttc_discord_interaction_free(ttc_discord_interaction_t *interaction) {
	switch (interaction->type) {
		case DiscordInteractionAppCmd:
			for (size_t ind = 0; ind < interaction->data.command->opt_count; ind++) {
				uint32_t type = interaction->data.command->options[ind].type;
				if (type == DiscordOptionString || type == DiscordOptionChannel ||
						type == DiscordOptionUser || type == DiscordOptionRole) {
					free(interaction->data.command->options[ind].value.string);
				}
			}
			free(interaction->data.command->options);
			free(interaction->data.command);
			break;
		case DiscordInteractionModalSubmit:
			for (size_t ind = 0; ind < interaction->data.modal->field_count; ind++) {
				free(interaction->data.modal->fields[ind].value);
				free(interaction->data.modal->fields[ind].id);
			}
			free(interaction->data.modal->id);
			free(interaction->data.modal);
			break;
		case DiscordInteractionMsgComponent: {
			ttc_discord_component_data_t *component = interaction->data.component;
			if (component->type != DiscordComponentButton ||
					component->type != DiscordComponentTextInput ||
					component->type != DiscordComponentActionRow) {
				for (size_t index = 0; index < component->count; index++) {
					free(component->values[index]); /*Free each string*/
				}
				free(component->values); /*free the pointers*/
			}

			free(component->id);
			free(component);
		} break;
		default:
			break;
	}

	ttc_discord_member_free(interaction->member);
	free(interaction->token);
	free(interaction);
}

ttc_discord_member_t *ttc_discord_member_json_to_struct(json_object *member) {
	json_object *user, *array, *object;
	json_bool found;
	ttc_discord_member_t *output;

	output = calloc(1, sizeof(ttc_discord_member_t));

	found = json_object_object_get_ex(member, "user", &user);
	output->user.id = strtoull(json_object_get_string(json_object_object_get(user, "id")), NULL, 10);
	output->permission =
			strtoull(json_object_get_string(json_object_object_get(member, "permissions")), NULL, 10);

	/*we are storing the roles as strings as we would then need
	 * reparse them into strings when working with them.
	 * and would need to parse the roles a user selects
	 * and it just gets to be a lot of parsing
	 */
	json_object_object_get_ex(member, "roles", &array);
	output->role_count = json_object_array_length(array);

	/*allocate pointers for all the string dups*/
	if (output->role_count) {
		output->roles = calloc(output->role_count, sizeof(char *));

		for (size_t index = 0; index < output->role_count; index++) {
			object = json_object_array_get_idx(array, index);
			output->roles[index] = strdup(json_object_get_string(object));
		}
	}

	return output;
}

ttc_discord_interaction_t *ttc_discord_interaction_to_struct(json_object *interaction) {
	json_object *object, *member;
	json_bool found;
	ttc_discord_interaction_t *output;

	TTC_LOG_DEBUG("%s\n", json_object_to_json_string(interaction));
	output = calloc(1, sizeof(ttc_discord_interaction_t));

	found = json_object_object_get_ex(interaction, "id", &object);
	if (found == false) {
		/* ID is required so this seems like this isnt a real
		 * interaction but some other json object given by mistake
		 */
		free(output);
		return NULL;
	}

	output->id = strtoull(json_object_get_string(object), NULL, 10);

	found = json_object_object_get_ex(interaction, "application_id", &object);
	output->app_id = strtoull(json_object_get_string(object), NULL, 10);

	found = json_object_object_get_ex(interaction, "app_permissions", &object);
	if (found) {
		output->app_permission = strtoull(json_object_get_string(object), NULL, 10);
	}

	json_object_object_get_ex(interaction, "type", &object);
	output->type = json_object_get_int64(object);

	json_object_object_get_ex(interaction, "data", &object);
	switch (output->type) {
		case DiscordInteractionPing:
			break;
		case DiscordInteractionAppCmd:
			output->data.command = ttc_discord_interaction_resolve_app_cmd_data(object);
			break;
		case DiscordInteractionModalSubmit:
			output->data.modal = ttc_discord_interaction_resolve_modal(object);
			break;
		case DiscordInteractionMsgComponent:
			output->data.component = ttc_discord_interaction_resolve_component(object);
		default:
			break;
	}

	found = json_object_object_get_ex(interaction, "guild_id", &object);
	output->guild_id = found ? strtoull(json_object_get_string(object), NULL, 10) : 0;
	if (output->guild_id) {
		found = json_object_object_get_ex(interaction, "member", &member);
		output->member = ttc_discord_member_json_to_struct(member);
	}

	found = json_object_object_get_ex(interaction, "channel", &object);
	output->channel = NULL; /*TODO:*/

	found = json_object_object_get_ex(interaction, "channel_id", &object);
	output->channel_id = found ? strtoull(json_object_get_string(object), NULL, 10) : 0;

	/*TODO USER/MEMBER*/

	json_object_object_get_ex(interaction, "token", &object);
	output->token = strdup(json_object_get_string(object));

	json_object_object_get_ex(interaction, "version", &object);
	output->version = json_object_get_int(object);

	/*TODO message*/

	/*TODO
	 * App Perms
	 * Locale
	 * Guild Locale
	 */

	/*TODO entitlements but i don't want to
	 * I'd rather make an OF than get payed by
	 * Discord apps :P
	 */

	return output;
}

void handle_interaction(json_object *object, ttc_discord_ctx_t *ctx) {
	ttc_discord_interaction_t *interaction;
	char *url;

	TTC_LOG_DEBUG("%s\n", json_object_to_json_string(object));
	interaction = ttc_discord_interaction_to_struct(object);

	CREATE_SNPRINTF_STRING(url, "/api/v10/interactions/%" PRIu64 "/%s/callback", interaction->id,
												 interaction->token);

	switch (interaction->type) {
		case DiscordInteractionAppCmd:
			handle_interaction_app_command(interaction, ctx, url);
			break;
		case DiscordInteractionMsgComponent:
			handle_interaction_msg_component(interaction, ctx, url);
			break;
		case DiscordInteractionModalSubmit:
			handle_interaction_modal_submit(interaction, ctx, url);
			break;
		default:
			TTC_LOG_WARN("Unknown interaction type: %d\n", interaction->type);
	}

	ttc_discord_interaction_free(interaction);
	free(url);
}

void handle_dispatch(json_object *json_response, ttc_discord_ctx_t *ctx) {
	json_object *sequence, *type, *d, *resumeurl, *sessionid;
	const char *type_str;

	sequence = json_object_object_get(json_response, "s");
	ctx->sequence = json_object_get_int64(sequence);

	type = json_object_object_get(json_response, "t");
	type_str = json_object_get_string(type);

	if (strcmp("READY", type_str) == 0) {
		d = json_object_object_get(json_response, "d");
		resumeurl = json_object_object_get(d, "resume_gateway_url");
		sessionid = json_object_object_get(d, "session_id");

		free(ctx->resume_url);
		free(ctx->session_id);
		ctx->resume_url = strdup(&json_object_get_string(resumeurl)[6]);
		ctx->session_id = strdup(json_object_get_string(sessionid));

		TTC_LOG_DEBUG("Resume URL: %s\n", ctx->resume_url);
		TTC_LOG_DEBUG("Session ID: %s\n", ctx->session_id);
	} else if (strcmp("INTERACTION_CREATE", type_str) == 0) {
		d = json_object_object_get(json_response, "d");
		handle_interaction(d, ctx);
	} else {
		TTC_LOG_WARN("Unknown dispatch event: %s\n", type_str);
	}
}

void discord_handle_hello(json_object *response, ttc_discord_ctx_t *ctx) {
	json_object *data, *heartbeat_interval;

	data = json_object_object_get(response, "d");
	heartbeat_interval = json_object_object_get(data, "heartbeat_interval");

	ctx->heart_interval = json_object_get_uint64(heartbeat_interval);
	TTC_LOG_DEBUG("Discord Hello says to beat heart every %lu milliseconds\n", ctx->heart_interval);
	pthread_create(&ctx->heart_thread, NULL, discord_heart, ctx);
}

void discord_polite_exit() {
}

void discord_ws_closed(uint16_t close_code, ttc_discord_ctx_t *ctx) {
	switch (close_code) {
		case TtcWsCloseNormal:
		case TtcWsGoingAway:
			discord_reconnect(ctx);
			break;
		default:
			TTC_LOG_WARN("Unknown Close code: %d\n", close_code);
			pthread_cancel(ctx->heart_thread);
			pthread_exit(NULL);
	}
}

void parse_message(ttc_ws_buffer_t *buffer, ttc_discord_ctx_t *ctx) {
	json_object *response = json_tokener_parse(buffer->data);
	json_object *op = json_object_object_get(response, "op");
	int opint = json_object_get_int(op);

	switch (opint) {
		case DiscordDispatch: {
			handle_dispatch(response, ctx);
			break;
		}
		case DiscordReconnect: {
			discord_reconnect(ctx);
			break;
		}
		case DiscordInvalidSession: {
			TTC_LOG_DEBUG("Discord session invalidated creating new session\n%s\n", buffer->data);
			ttc_ws_free(ctx->gateway);
			ctx->gateway = ttc_ws_create_from_host(ctx->gateway_url, "443", ctx->ssl_ctx);
			if (!ctx->gateway) {
				pthread_cancel(ctx->heart_thread);
				pthread_exit(exit);
			}
			break;
		}
		case DiscordHello: {
			discord_handle_hello(response, ctx);
			break;
		}
		case DiscordHeartbeatACK: {
			break;
		}
		default: {
			/*There shouldn't be any other event types :/*/
			TTC_LOG_DEBUG("Server sent an unknown message to us\n%s\n", buffer->data);
		}
	}

	json_object_put(response);
}

void *discord_gateway_read(void *vargp) {
	ttc_discord_ctx_t *ctx = vargp;
	ttc_ws_buffer_t *buffer;

	while (1) {
		buffer = ttc_ws_read(ctx->gateway);

		switch (buffer->opcode) {
			case 0: {                 /*the websocket closed without telling us*/
				discord_reconnect(ctx); /*To reconnect*/
				break;
			}
			case TTC_WS_TEXT_FRAME: {
				parse_message(buffer, ctx);
				break;
			}
			case TTC_WS_CONN_CLOSE_FRAME: {
				TTC_LOG_DEBUG("Discord closed frame with code: %d\n", buffer->close_code);
				/*Parse the buffer and decide if we can reconnect*/
				discord_ws_closed(buffer->close_code, ctx);
				break;
			}
			default: {
				TTC_LOG_WARN("Unknown opcode: %d\n", buffer->opcode);
			}
		}

		ttc_ws_buffer_free(buffer);
	}
	pthread_exit(NULL);
}

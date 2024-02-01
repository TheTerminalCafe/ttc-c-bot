#include "ttc-discord/api.h"
#include "ttc-discord/gateway.h"
#include <stdint.h>
#include <ttc-discord/interaction.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/moderation.h>
#include <ttc-log.h>

void echo_handle(ttc_discord_interaction_t *interaction, 
		ttc_discord_ctx_t *ctx, const char *url) {
	ttc_discord_modal_t modal;
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	json_object *object, *data, *message, *type;
	ttc_discord_app_cmd_opt_t *option;
	char *channel_id, *message_id;
	ttc_discord_embed_t embed = {0};
	uint64_t mid, cid; 

	channel_id = NULL;
	message_id = NULL;

	for(size_t ind = 0; ind < interaction->data.command->opt_count; ind++) {
		option = &interaction->data.command->options[ind];
		if(strcmp("channel", option->name) == 0) {
			channel_id = option->value.string;
		} else if(strcmp("message", option->name) == 0) {
			message_id = option->value.string;
		} 
	}
	if(message_id && channel_id) {
		mid = strtoull(message_id, NULL, 10);
		if(!mid) {
			ttc_discord_interaction_respond_embed(ctx, "Message NaN", "Message Id needs to be a number!", 0xf80000, url);
			return;
		}
		if (ttc_discord_message_extract_embed(ctx, strtoull(channel_id, NULL, 10), mid, &embed)) {
			ttc_discord_interaction_respond_embed(ctx, "I'm Not the Author/message Not found", "I didn't author this message can't proceed\n"
					"Or we where unable to find a message with this ID\n", 0xf80000, url);
			return;
		}
	}

	modal.id = "embed_modal";
	modal.field_count = 5;
	modal.name = "Create Embed";
	modal.fields[0].id = "embed_title";
	modal.fields[0].label = "Embed title:";
	modal.fields[0].type = DiscordComponentTextInput;
	modal.fields[0].style = DiscordTextInputSingleLine;
	modal.fields[0].value = embed.title ? embed.title : "";
	modal.fields[0].required = true;

	modal.fields[1].id = "embed_desc";
	modal.fields[1].label = "Embed Desc:";
	modal.fields[1].type = DiscordComponentTextInput;
	modal.fields[1].style = DiscordTextInputParagraph;
	modal.fields[1].value = embed.description ? embed.description : "";
	modal.fields[1].required = true;
	
	modal.fields[2].id = "embed_channel";
	modal.fields[2].label = "Channel ID:";
	modal.fields[2].type = DiscordComponentTextInput;
	modal.fields[2].style = DiscordTextInputSingleLine;
	modal.fields[2].value = channel_id ? channel_id : "";
	modal.fields[2].required = true;
	
	modal.fields[3].id = "embed_color";
	modal.fields[3].label = "Color(0xff8080):";
	modal.fields[3].type = DiscordComponentTextInput;
	modal.fields[3].style = DiscordTextInputSingleLine;
	modal.fields[3].required = false;
	modal.fields[3].value = "0x000000";
	
	modal.fields[4].id = "old_message";
	modal.fields[4].label = "Message ID To Edit:";
	modal.fields[4].type = DiscordComponentTextInput;
	modal.fields[4].style = DiscordTextInputSingleLine;
	modal.fields[4].required = false;
	modal.fields[4].value = message_id ? message_id : "";

	object = ttc_discord_form_to_json(&modal);
	type = json_object_new_int(DiscordInteractionCallbackModal);
	data = json_object_new_object();
	message = json_object_new_object();

	json_object_object_add(message, "type", type);
	json_object_object_add(message, "data", object);

	TTC_LOG_WARN("JSON_STRING: %s\n", json_object_to_json_string(message));

	request = ttc_http_new_request();
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	ttc_http_request_set_path(request, url);
	
	response = ttc_discord_api_send_json(ctx, request, message);	
	
	TTC_LOG_WARN("response to echo: %d\n%s\n", response->status,response->data);
	ttc_http_request_free(request);
	ttc_http_response_free(response);
	json_object_put(data);
}

void userinfo_handle(ttc_discord_interaction_t *interaction, 
		ttc_discord_ctx_t *ctx, const char *url) {
	
}

void kick_handle(ttc_discord_interaction_t *interaction, 
		ttc_discord_ctx_t *ctx, const char *url) {
	ttc_discord_app_cmd_opt_t *option;
	uint64_t user_id = 0;
	uint32_t bot_position, target_position, caller_position;
	char buffer[101]; /*description field is limited to 100 chars*/
	int result;
	size_t ind;
	/*Default to no reason*/
	char *reason = "No Reason Given";


	/*Discord API says we should responsed within 3 seconds.
	 * respond with a loading status and follow up later*/
	ttc_discord_interaction_loading(ctx, url);

	if(interaction->guild_id == 0) {
		TTC_LOG_WARN("Guild id not set was this ban sent in a DM?\n");
		ttc_discord_interaction_loading_respond(ctx, "This is a DM?", "You can't ban us from DMS!",
				0xff0000, interaction);
		return;
	}

	/*Look for optional arguments we support*/
	for(size_t ind = 0; ind < interaction->data.command->opt_count; ind++) {
		option = &interaction->data.command->options[ind];
		if(strcmp("user", option->name) == 0) {
			user_id = strtoull(option->value.string, NULL, 10);
		} else if(strcmp("reason", option->name) == 0) {
			reason = option->value.string;
		}
	}

	if(!(interaction->member.permission & (DISCORD_PERMISSION_KICK | DISCORD_PERMISSION_ADMIN))) {
		TTC_LOG_WARN("User <@%lu> does not have kick perms\n", interaction->member.user.id);
		ttc_discord_interaction_loading_respond(ctx, "Permission Denied!", "You don't have kick perms for this GUILD!", 0xff0000, interaction);
		return;
	}

	if(user_id == 0) {
		ttc_discord_interaction_loading_respond(ctx, "Invalid User", "You provided an invalid user ID", 0xf80000, interaction);
		TTC_LOG_WARN("No user id provided or invalid ID given\n");
		return;
	}


	if(user_id == interaction->member.user.id) {
		TTC_LOG_WARN("User tried to ban themselves\n");
		ttc_discord_interaction_loading_respond(ctx, "No kicking yourself!", "Looks like you tried to kick yourself!", 0xff0000, interaction);
		return;
	}

	target_position = discord_get_user_position(ctx, interaction->guild_id, user_id);	
	caller_position = discord_get_user_position(ctx, interaction->guild_id, interaction->member.user.id);
	bot_position = discord_get_user_position(ctx, interaction->guild_id, interaction->app_id);
	
	if(caller_position <= target_position) {
		ttc_discord_interaction_loading_respond(ctx, "You can't do this", "You can't kick this user as their permissions are the same as or greater than yours", 0xf80000, interaction);
		return;
	}

	if(bot_position <= target_position) {
		ttc_discord_interaction_loading_respond(ctx, "I can't do this", "Sorry the bot is of a lower permission level than this person", 0xf80000, interaction);
		return;
	}

	result = ttc_discord_kick_member(ctx, user_id, interaction->guild_id, reason);

	TTC_LOG_DEBUG("kick HTTP status code: %d\n", result);
	
	if(result == 204) {
		char *fmt = "The user <@%lu> has been kicked!\n";
		snprintf(buffer, 101, fmt, user_id);

		ttc_discord_interaction_loading_respond(ctx, "User kicked!", buffer,
				0x00ff00, interaction);
	} else {
		char *fmt = "The user <@%lu> couldn't be kicked HTTP status: %d";
		snprintf(buffer, 101, fmt, user_id, result);
		ttc_discord_interaction_loading_respond(ctx, "Unable to kicked user!", buffer,
				0xff0000, interaction);
	}
}

void pardon_handle(ttc_discord_interaction_t *interaction, 
		ttc_discord_ctx_t *ctx, const char *url) {
	ttc_discord_app_cmd_opt_t *option;
	uint64_t user_id = 0;
	uint32_t seconds, bot_position, target_position, caller_position;
	char buffer[101]; /*description field is limited to 100 chars*/
	int result;
	size_t ind;
	/*Default to no reason*/
	char *reason = "No Reason Given";

	seconds = 0;

	/*Discord API says we should responsed within 3 seconds.
	 * respond with a loading status and follow up later*/
	ttc_discord_interaction_loading(ctx, url);

	if(interaction->guild_id == 0) {
		TTC_LOG_WARN("Guild id not set was this command sent in a DM?\n");
		ttc_discord_interaction_loading_respond(ctx, "This is a DM?", "You can't call this command in DMS!",
				0xff0000, interaction);
		return;
	}

	/*Look for optional arguments we support*/
	for(size_t ind = 0; ind < interaction->data.command->opt_count; ind++) {
		option = &interaction->data.command->options[ind];
		if(strcmp("user", option->name) == 0) {
			user_id = strtoull(option->value.string, NULL, 10);
		} else if(strcmp("reason", option->name) == 0) {
			reason = option->value.string;
		}
	}

	if(!(interaction->member.permission & (DISCORD_PERMISSION_BAN | DISCORD_PERMISSION_ADMIN))) {
		TTC_LOG_WARN("User <@%lu> does not have unban perms\n", interaction->member.user.id);
		ttc_discord_interaction_loading_respond(ctx, "Permission Denied!", "You don't have unban perms for this GUILD!", 0xff0000, interaction);
		return;
	}

	if(user_id == 0) {
		ttc_discord_interaction_loading_respond(ctx, "Invalid User", "You provided an invalid user ID", 0xf80000, interaction);
		TTC_LOG_WARN("No user id provided or invalid ID given\n");
		return;
	}

	result = ttc_discord_pardon_member(ctx, user_id, interaction->guild_id, reason);

	if(result == 204) {
		char *fmt = "The user <@%lu> has been unbanned!\n";
		snprintf(buffer, 101, fmt, user_id);

		ttc_discord_interaction_loading_respond(ctx, "User unbanned!", buffer,
				0x00ff00, interaction);
	} else {
		char *fmt = "The user <@%lu> couldn't be unbanned HTTP status: %d";
		snprintf(buffer, 101, fmt, user_id, result);
		ttc_discord_interaction_loading_respond(ctx, "Unable to unban user!", buffer,
				0xff0000, interaction);
	}
}


void ban_handle(ttc_discord_interaction_t *interaction, 
		ttc_discord_ctx_t *ctx, const char *url) {
	ttc_discord_app_cmd_opt_t *option;
	uint64_t user_id = 0;
	uint32_t seconds, bot_position, target_position, caller_position;
	char buffer[101]; /*description field is limited to 100 chars*/
	int result;
	size_t ind;
	/*Default to no reason*/
	char *reason = "No Reason Given";

	seconds = 0;

	/*Discord API says we should responsed within 3 seconds.
	 * respond with a loading status and follow up later*/
	ttc_discord_interaction_loading(ctx, url);

	if(interaction->guild_id == 0) {
		TTC_LOG_WARN("Guild id not set was this ban sent in a DM?\n");
		ttc_discord_interaction_loading_respond(ctx, "This is a DM?", "You can't ban us from DMS!",
				0xff0000, interaction);
		return;
	}

	/*Look for optional arguments we support*/
	for(size_t ind = 0; ind < interaction->data.command->opt_count; ind++) {
		option = &interaction->data.command->options[ind];
		if(strcmp("user", option->name) == 0) {
			user_id = strtoull(option->value.string, NULL, 10);
		} else if(strcmp("seconds", option->name) == 0) {
			seconds = option->value.integer;
		} else if(strcmp("reason", option->name) == 0) {
			reason = option->value.string;
		}
	}

	if(!(interaction->member.permission & (DISCORD_PERMISSION_BAN | DISCORD_PERMISSION_ADMIN))) {
		TTC_LOG_WARN("User <@%lu> does not have ban perms\n", interaction->member.user.id);
		ttc_discord_interaction_loading_respond(ctx, "Permission Denied!", "You don't have ban perms for this GUILD!", 0xff0000, interaction);
		return;
	}

	if(user_id == 0) {
		ttc_discord_interaction_loading_respond(ctx, "Invalid User", "You provided an invalid user ID", 0xf80000, interaction);
		TTC_LOG_WARN("No user id provided or invalid ID given\n");
		return;
	}


	if(user_id == interaction->member.user.id) {
		TTC_LOG_WARN("User tried to ban themselves\n");
		ttc_discord_interaction_loading_respond(ctx, "No banning oneself!", "Looks like you tried to ban yourself!", 0xff0000, interaction);
		return;
	}

	target_position = discord_get_user_position(ctx, interaction->guild_id, user_id);	
	caller_position = discord_get_user_position(ctx, interaction->guild_id, interaction->member.user.id);
	bot_position = discord_get_user_position(ctx, interaction->guild_id, interaction->member.user.id);
	
	if(caller_position <= target_position) {
		ttc_discord_interaction_loading_respond(ctx, "You can't do this", "You can't ban this user as their permissions are the same as or greater than yours", 0xf80000, interaction);
		return;
	}

	if(bot_position <= target_position) {
		ttc_discord_interaction_loading_respond(ctx, "I can't do this", "Sorry the bot is of a lower permission level than this person", 0xf80000, interaction);
		return;
	}

	TTC_LOG_DEBUG("Ban HTTP status code: %d\n", result);
	result = ttc_discord_ban_member(ctx, user_id, interaction->guild_id, reason, seconds);

	if(result == 204) {
		char *fmt = "The user <@%lu> has been banned!\n";
		snprintf(buffer, 101, fmt, user_id);

		ttc_discord_interaction_loading_respond(ctx, "User banned!", buffer,
				0x00ff00, interaction);
	} else {
		char *fmt = "The user <@%lu> couldn't be banned HTTP status: %d";
		snprintf(buffer, 101, fmt, user_id, result);
		ttc_discord_interaction_loading_respond(ctx, "Unable to ban user!", buffer,
				0xff0000, interaction);
	}
}

static void generate_iso8601_string(char* result, time_t time_unix) {
	struct tm time_tm;
	gmtime_r(&time_unix, &time_tm);
	snprintf(result, 21, "%04d-%02d-%02dT%02d:%02d:%02dZ", time_tm.tm_year + 1900,
					 time_tm.tm_mon + 1, time_tm.tm_mday, time_tm.tm_hour, time_tm.tm_min,
					 time_tm.tm_sec);
}

static void add_time_to_unix_timestamp(time_t* time, int seconds, int minutes,
																			 int hours, int days) {
	*time += seconds + 60 * minutes + 3600 * hours + 86400 * days;
}

void timeout_handle(ttc_discord_interaction_t* interaction,
										ttc_discord_ctx_t* ctx, const char* url) {

	ttc_discord_interaction_loading(ctx, url);

	uint64_t target_user = 0;
	uint8_t days;
	int64_t days_raw;
	char* reason = NULL;

	ttc_discord_app_cmd_data_t* command = interaction->data.command;
	for (size_t i = 0; i < command->opt_count; ++i) {
		ttc_discord_app_cmd_opt_t* option = &command->options[i];
		GET_COMMAND_ARGUMENT_USER(option, "user", target_user);
		GET_COMMAND_ARGUMENT_INT(option, "days_to_timeout", days_raw);
		GET_COMMAND_ARGUMENT_STRING(option, "reason", reason);
	}

	// TODO: Add permission checks!

	if (days_raw > 28 || days_raw < 0) {
		ttc_discord_interaction_loading_respond(ctx, "Unable to timeout user!",
																						"You provided either less than 0 days or more than 28 days",
																						0xff0000, interaction);
		return;
	}
	days = (uint8_t) days_raw;

	time_t target_time = time(NULL);
	add_time_to_unix_timestamp(&target_time, 0, 0, 0, days);

	char time_str[21];
	generate_iso8601_string(time_str, target_time);

	int result = ttc_discord_timeout_member(
			ctx, target_user, interaction->guild_id, time_str, reason);
	if (result == 200) {
		char buf[101];
		snprintf(buf, 101,
						 "The user <@%" PRIu64 "> is timeouted for %" PRIu8 " days",
						 target_user, days);

		ttc_discord_interaction_loading_respond(ctx, "User timed out!", buf,
																						0x00ff00, interaction);
	} else {
		char buf[101];
		snprintf(buf, 101,
						 "The user <@%" PRIu64
						 "> is couldn't be timeouted. HTTP response: %d",
						 target_user, result);

		ttc_discord_interaction_loading_respond(ctx, "Unable to timeout user!", buf,
																						0xff0000, interaction);
	}
}

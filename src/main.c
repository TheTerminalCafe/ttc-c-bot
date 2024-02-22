#include <json-c/json_object.h>
#include <json-c/json_tokener.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ttc-discord/api.h>
#include <ttc-discord/commands.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-discord/interaction.h>
#include <ttc-discord/moderation.h>
#include <ttc-discord/ui.h>
#include <ttc-log.h>

#include "command.h"
#include "components.h"
#include "modals.h"

static command_opt_t echo_opts[] = {
		{.name = "channel",
		 .description = "Channel message is from",
		 .required = 0,
		 .type = DiscordOptionChannel},
		{.name = "message",
		 .description = "Message to edit",
		 .required = 0,
		 .type = DiscordOptionString},
};

static command_t echo = {.name = "echo",
												 .description = "Echos it's input",
												 .type = 1,
												 .options = echo_opts,
												 .option_count = 2,
												 .allow_in_dms = false,
												 .default_permissions = DISCORD_PERMISSION_ADMIN};

static command_opt_t userinfo_opts = {.name = "User",
																			.description = "User to get info for",
																			.type = DiscordOptionUser,
																			.required = 1};
static command_t userinfo = {.name = "userinfo",
														 .description = "get a users info",
														 .type = 1,
														 .option_count = 1,
														 .options = &userinfo_opts,
														 .allow_in_dms = false,
														 .default_permissions = DISCORD_PERMISSION_ADMIN};

static command_opt_t ban_opts[] = {
		{.name = "user", .description = "user to ban", .type = DiscordOptionUser, .required = 1},
		{.name = "reason", .description = "reason", .type = DiscordOptionString, .required = 0},
		{.name = "seconds",
		 .description = "seconds to delete of messages",
		 .type = DiscordOptionInteger,
		 .required = 0},
};
static command_t ban = {.name = "ban",
												.description = "kick user",
												.type = 1,
												.options = ban_opts,
												.option_count = 3,
												.allow_in_dms = false,
												.default_permissions = DISCORD_PERMISSION_ADMIN | DISCORD_PERMISSION_BAN};

static command_opt_t kick_opts[] = {
		{.name = "user", .description = "user to kick", .type = DiscordOptionUser, .required = 1},
		{.name = "reason",
		 .description = "reason for kick",
		 .type = DiscordOptionString,
		 .required = 0},
};

static command_t kick = {.name = "kick",
												 .description = "kick user",
												 .type = 1,
												 .options = kick_opts,
												 .option_count = 2,
												 .allow_in_dms = false,
												 .default_permissions = DISCORD_PERMISSION_ADMIN | DISCORD_PERMISSION_BAN |
																								DISCORD_PERMISSION_KICK};

static command_opt_t pardon_opts[] = {
		{.name = "user", .description = "user to pardon", .type = DiscordOptionUser, .required = 1},
		{.name = "reason",
		 .description = "reason for pardon",
		 .type = DiscordOptionString,
		 .required = 0},
};

static command_t pardon = {.name = "pardon",
													 .description = "unban user",
													 .type = 1,
													 .options = pardon_opts,
													 .option_count = 2,
													 .allow_in_dms = false,
													 .default_permissions =
															 DISCORD_PERMISSION_ADMIN | DISCORD_PERMISSION_BAN};

static command_opt_t timeout_opts[] = {
		{.name = "user", .description = "user to timeout", .type = DiscordOptionUser, .required = 1},
		{.name = "days",
		 .description = "number of days to timeout",
		 .type = DiscordOptionInteger,
		 .required = 0},
		{.name = "hours",
		 .description = "number of hours to timeout",
		 .type = DiscordOptionInteger,
		 .required = 0},
		{.name = "minutes",
		 .description = "number of minutes to timeout",
		 .type = DiscordOptionInteger,
		 .required = 0},
		{.name = "seconds",
		 .description = "number of seconds to timeout",
		 .type = DiscordOptionInteger,
		 .required = 0},
		{.name = "reason",
		 .description = "reason for timeout",
		 .type = DiscordOptionString,
		 .required = 0}};

static command_t timeout = {.name = "timeout",
														.description = "timeout member",
														.type = 1,
														.options = timeout_opts,
														.option_count = 6,
														.allow_in_dms = false,
														.default_permissions = DISCORD_PERMISSION_ADMIN |
																									 DISCORD_PERMISSION_BAN |
																									 DISCORD_PERMISSION_KICK};

int ttc_discord_create_text_input(ttc_discord_ctx_t *ctx, uint32_t type, const char *menu_id,
																	uint64_t channel);

int main() {
	ttc_log_set_level(TtcLogAll);
	ttc_log_init_file("log.txt");
	ttc_discord_ctx_t *discord = ttc_discord_ctx_create("config.ini");
	if (!discord) {
		ttc_log_deinit_file();
		return 1;
	}

	discord_create_application_command(&echo, discord, echo_handle);
	discord_create_application_command(&kick, discord, kick_handle);
	discord_create_application_command(&pardon, discord, pardon_handle);
	discord_create_application_command(&ban, discord, ban_handle);
	discord_create_application_command(&timeout, discord, timeout_handle);

	ttc_discord_create_select_menu(discord, 6, "role_select", 913091622592458833, 25);

	ttc_discord_add_modal_listener(discord, "embed_modal", ttc_embed_modal_submit);
	ttc_discord_add_component_listener(discord, "role_select", ttc_self_roles_picked);

	ttc_discord_run(discord);

	ttc_discord_ctx_destroy(discord);
	ttc_log_deinit_file();
	return 0;
}

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include <ttc-discord/discord.h>
#include <ttc-discord/api.h>
/* Returns true if command was called in a guild.
 * False if it wasnt
 */
bool ttc_discord_command_in_guild(uint64_t gid);

void ttc_discord_interaction_respond_embed(ttc_discord_ctx_t *ctx, char *title, char *description, uint32_t color, const char *url);
void ttc_discord_interaction_loading(ttc_discord_ctx_t *ctx, const char *url);
void ttc_discord_interaction_loading_respond(ttc_discord_ctx_t *ctx, char *title,
		char *description, uint32_t color, ttc_discord_interaction_t *interaction);

/**
 * @brief extract string from inside option loop
 * @param option this takes a ttc_discord_app_cmd_opt_t pointer
 * @param searched_name this is how the field from the command is called
 * @param result_var name of the variable where the result should be stored
 */
#define GET_COMMAND_ARGUMENT_STRING(option, searched_name, result_var)                             \
	do {                                                                                             \
		if (!strcmp(option->name, searched_name)) {                                                    \
			result_var = option->value.string;                                                           \
		}                                                                                              \
	} while (0)

/**
 * @brief extract integer from inside option loop
 * @param option this takes a ttc_discord_app_cmd_opt_t pointer
 * @param searched_name this is how the field from the command is called
 * @param result_var name of the variable where the result should be stored
 */
#define GET_COMMAND_ARGUMENT_INT(option, searched_name, result_var)                                \
	do {                                                                                             \
		if (!strcmp(option->name, searched_name)) {                                                    \
			result_var = option->value.integer;                                                          \
		}                                                                                              \
	} while (0)

/**
 * @brief extract user snowflake from inside option loop
 * @param option this takes a ttc_discord_app_cmd_opt_t pointer
 * @param searched_name this is how the field from the command is called
 * @param result_var name of the variable where the result should be stored
 */
#define GET_COMMAND_ARGUMENT_USER(option, searched_name, result_var)                               \
	do {                                                                                             \
		if (!strcmp(option->name, searched_name)) {                                                    \
			result_var = (snowflake_t) strtoull(option->value.string, NULL, 10);                         \
		}                                                                                              \
	} while (0)

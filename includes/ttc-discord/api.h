#pragma once

#include <json-c/json.h>
#include <stdint.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>

#include <ttc-http/request.h>
#include <ttc-http/response.h>

/* all API Calls here return one of two types of codes.
 * negative return codes mean an application error.
 * positive return codes mean a HTTP status code.
 */

typedef enum {
	DiscordComponentActionRow = 1,
	DiscordComponentButton = 2,
	DiscordComponentStringSelect = 3,
	DiscordComponentTextInput = 4,
	DiscordComponentUserSelect = 5,
	DiscordCompenentRoleSelect = 6,
	DiscordComponentMentionableSelect = 7,
	DiscordComponentChannelSelect = 8
} DISCORD_COMPONENT_TYPE;

typedef enum {
	DiscordTextInputSingleLine = 1,
	DiscordTextInputParagraph = 2
} DISCORD_TEXT_INPUT_STYLES;

typedef enum {
	DiscordButtonPrimary = 1,
	DiscordButtonSecondary = 2,
	DiscordButtonSuccess = 3,
	DiscordButtonDanger = 4,
	DiscordButtonLink = 5
} DISCORD_BUTTON_STYLES;

#include <inttypes.h>
#include <stdio.h>
/**
 * @brief create a string on the heap via double snprintf call
 *
 * @param result pointer of where the result should be stored
 * @param format format string
 * @param ... optional arguments for the format string
 */
#define CREATE_SNPRINTF_STRING(result, format, ...)                                                \
	do {                                                                                             \
		int _length = snprintf(NULL, 0, format, __VA_ARGS__);                                          \
		result = calloc(1, _length + 1);                                                               \
		if (result) {                                                                                  \
			snprintf(result, _length + 1, format, __VA_ARGS__);                                          \
		}                                                                                              \
	} while (0)

int ttc_discord_message_extract_embed(ttc_discord_ctx_t *ctx, snowflake_t cid, snowflake_t mid,
																			ttc_discord_embed_t *embed);
json_object *ttc_discord_form_to_json(ttc_discord_modal_t *modal);

json_object *discord_get_guild_member(ttc_discord_ctx_t *ctx, uint64_t gid, uint64_t uid);
int discord_get_user_position(ttc_discord_ctx_t *ctx, uint64_t gid, uint64_t uid);

json_object *ttc_discord_embed_to_json(ttc_discord_embed_t *embed);

int discord_user_role(ttc_discord_ctx_t *ctx, uint64_t gid, uint64_t uid, uint64_t rid,
											const char *method);
ttc_http_response_t *ttc_discord_api_send_request(ttc_discord_ctx_t *ctx,
																									ttc_http_request_t *request);
ttc_http_response_t *ttc_discord_api_send_json(ttc_discord_ctx_t *ctx, ttc_http_request_t *request,
																							 json_object *message);

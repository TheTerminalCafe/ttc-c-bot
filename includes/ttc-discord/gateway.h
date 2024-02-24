#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <ttc-discord/gateway.h>

#include <json-c/json.h>

#include <ttc-discord/discord.h>

#define DISCORD_COMMAND_NAME_LENGTH 32
#define DISCORD_OPTION_NAME_LENGTH 32

typedef enum {
	DiscordInteractionPing = 1,
	DiscordInteractionAppCmd = 2,
	DiscordInteractionMsgComponent = 3,
	DiscordInteractionAppCmdAutoCmp = 4,
	DiscordInteractionModalSubmit = 5,
} DISCORD_INTERACTION_TYPE;

typedef enum {
	DiscordInteractionCallbackPong = 1,
	DiscordInteractionCallbackMessage = 4,
	DiscordInteractionCallbackDeferredMessage = 5,
	DiscordInteractionCallbackDeferredUpdateMessage = 6,
	DiscordInteractionCallbackUpdateMessage = 7,
	DiscordInteractionCallbackAppAutoCmpResult = 8,
	DiscordInteractionCallbackModal = 9,
	DiscordInteractionPremiumRequired = 10,
} DISCORD_INTERACTION_CALLBACK_TYPE;

typedef enum {
	DiscordUnknownError = 4000,
	/* Discord says to recconect. But should we?
	 * because if we sent an invalid payload? shouldn't we
	 * error and tell the user what payload was invalid
	 *
	 * Seems tricky cause if it's something to do with
	 * the bots setup we could just recconect. Get disconnected
	 * for an invalid payload then be recconect and that
	 * could eat a users 1000 daily connection limit.
	 *
	 * Maybe we implement a retry method so process goes like
	 * Disconnected(DiscordUnknownOpcode/DecodeError) ->
	 * Attempt recconect.
	 * if failed(for the same reason) give up
	 * else recconect.
	 */
	DiscordUnknownOpcode = 4001,
	DiscordDecodeError = 4002,
	DiscordNotAuthen = 4003,
	DiscordAuthFailed = 4004,
	DiscordAlreadyAuth = 4005,
	/*Discord skips 4006*/
	DiscordInvalidSeq = 4007,
	DiscordRateLimited = 4008,
	DiscordSessionTimeout = 4009,
	DiscordInvalidShard = 4010,
	DiscordShardingReq = 4011,
	DiscordInvalidAPIVersion = 4012,
	DiscordInvalidIntents = 4013,
	DiscordDisallowedIntents = 4014,
} DISCORD_GATEWAY_STATUS_CODES;

typedef enum {
	DiscordDispatch = 0,
	DiscordHeartbeat = 1,
	DiscordIdentify = 2,
	DiscordPresence = 3,
	DiscordVoiceState = 4,
	DiscordResume = 6,
	DiscordReconnect = 7,
	DiscordRequestGuildMembers = 8,
	DiscordInvalidSession = 9,
	DiscordHello = 10,
	DiscordHeartbeatACK = 11,
} DISCORD_GATEWAY_OPCODES;

typedef enum {
	DiscordSubCommand = 1,
	DiscordSubCommandGroup = 2,
	DiscordOptionString = 3,
	DiscordOptionInteger = 4,
	DiscordOptionBool = 5,
	DiscordOptionUser = 6,
	DiscordOptionChannel = 7,
	DiscordOptionRole = 8,
	DiscordOptionMentionable = 9,
	DiscordOptionDouble = 10,
	DiscordOptionAttachment = 11,
} DISCORD_APP_COMMAND_OPTION_TYPES;

// TODO: Check whether something like the UINT64_C macro needs to be used
#define DISCORD_PERMISSION_INSTANT_INVITE 1 << 0
#define DISCORD_PERMISSION_KICK 1 << 1
#define DISCORD_PERMISSION_BAN 1 << 2
#define DISCORD_PERMISSION_ADMIN 1 << 3
#define DISCORD_PERMISSION_MANAGE_CHANNELS 1 << 4
#define DISCORD_PERMISSION_MANAGE_GUILD 1 << 5
#define DISCORD_PERMISSION_ADD_REACTION 1 << 6
#define DISCORD_PERMISSION_AUDIT_LOG 1 << 7
#define DISCORD_PERMISSION_PRIORITY_SPEAKER 1 << 8
#define DISCORD_PERMISSION_STREAM 1 << 9
#define DISCORD_PERMISSION_VIEW_CHANNEL 1 << 10
#define DISCORD_PERMISSION_SEND_MESSAGES 1 << 11
#define DISCORD_PERMISSION_SEND_TTS 1 << 12
#define DISCORD_PERMISSION_MANAGE_MESSAGE 1 << 13
#define DISCORD_PERMISSION_EMBED_LINKS 1 << 14
#define DISCORD_PERMISSION_ATTACH_FILES 1 << 15
#define DISCORD_PERMISSION_READ_HISTORY 1 << 16
#define DISCORD_PERMISSION_AT_EVERYONE 1 << 17
#define DISCORD_PERMISSION_EXTERNAL_EMOJIS 1 << 18
#define DISCORD_PERMISSION_GUILD_INSIGHTS 1 << 19
#define DISCORD_PERMISSION_CONNECT 1 << 20
#define DISCORD_PERMISSION_SPEAK 1 << 21
#define DISCORD_PERMISSION_MUTE 1 << 22
#define DISCORD_PERMISSION_DEAFEN 1 << 23
#define DISCORD_PERMISSION_MOVE_MEMBERS 1 << 24
#define DISCORD_PERMISSION_USE_VAD 1 << 25
#define DISCORD_PERMISSION_MODERATE_MEMBERS 1ULL << 40
/*TODO: the rest*/

typedef struct ttc_discord_user_s {
	snowflake_t id;
	char username[33]; /*discord usernames can be 32 chars long*/
										 /*TODO: The rest of user*/
} ttc_discord_user_t;

typedef struct ttc_discord_member_s {
	ttc_discord_user_t user;
	char *nickname;
	char *avatar;
	char **roles;
	size_t role_count;
	/*TODO: TIMESTAMPS*/
	bool deaf;
	bool mute;
	uint32_t flags;
	bool pending;

	uint64_t permission;
} ttc_discord_member_t;

typedef struct ttc_discord_app_cmd_opt_s ttc_discord_app_cmd_opt_t;

struct ttc_discord_app_cmd_opt_s {
	char name[DISCORD_OPTION_NAME_LENGTH + 1];
	uint32_t type;
	union {
		char *string;
		int64_t integer;
		double floating;
		bool boolean;
	} value;
	ttc_discord_app_cmd_opt_t *options; /*present if group or subcommand*/
	bool focused;
};

typedef struct ttc_discord_component_data {
	uint32_t type;
	char *id;
	size_t count;
	char **values; /*Technically if these where roles,
									*Channels or users ids we could store
									*them as uint 64 but string is also possible
									*/
} ttc_discord_component_data_t;

typedef struct ttc_discord_app_cmd_data {
	snowflake_t id;                             /**< Command id*/
	char name[DISCORD_COMMAND_NAME_LENGTH + 1]; /**Discord say 1-32 char length name so + 1 for NULL*/
	uint32_t type;
	void *resolved_data;                /*TODO*/
	ttc_discord_app_cmd_opt_t *options; /*array of options*/
	size_t opt_count;                   /**How many options present*/
	snowflake_t guildid;
	snowflake_t targetid;
} ttc_discord_app_cmd_data_t;

typedef struct ttc_discord_interaction_s {
	snowflake_t id;     /**Interaction id*/
	snowflake_t app_id; /**Id of app interaction is for*/
	DISCORD_INTERACTION_TYPE type;
	union {
		ttc_discord_component_data_t *component;
		ttc_discord_modal_t *modal;
		ttc_discord_app_cmd_data_t *command;
	} data;                 /*TODO OPTIONAL or rather will be NULL for Ping events*/
	snowflake_t guild_id;   /*OPTIONAL*/
	void *channel;          /*TODO OPTIONAL*/
	snowflake_t channel_id; /*OPTIONAL*/
	ttc_discord_member_t *member;
	ttc_discord_user_t user;
	char *token;             /**Interaction Token*/
	uint32_t version;        /**readonly discord says this is always 1*/
	void *message;           /**TODO: OPTIONAL for components message was attached to*/
	uint64_t app_permission; /**Bitwise set of perms of bot in channel OPTIONAL*/
	char *locale;            /**Users local not present on PING interactions*/
	char *guild_locale;      /**Guild locale if command called in guild*/
	void *entitlements;      /**Yucky discord montesitation*/
} ttc_discord_interaction_t;

ttc_discord_interaction_t *ttcdc_interaction_to_struct(json_object *interaction);
void discord_identify(ttc_discord_ctx_t *ctx);
void *discord_gateway_read(void *vargp);

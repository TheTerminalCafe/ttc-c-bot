#include <ttc-discord/discord.h>
#include <stdint.h>


/** @brief remove/kick a user from guild
 * @param ctx is a pointer to the discord context
 * @param uid is the user id of the account to kick
 * @param gid is the guild id to be kicked from
 * @param reason is the reason if there is one. OPTIONAL
 * 
 * returns a HTTP status code
 */
int ttc_discord_kick_member(ttc_discord_ctx_t *ctx, uint64_t uid, 
		uint64_t gid, char *reason);


/** @brief destroy a guild ban.
 * @param ctx is a pointer to the discord context
 * @param uid is the user id of the account to unban
 * @param gid is the guild id to be unbanned from
 * @param reason is the reason if there is one. OPTIONAL
 * 
 * returns a HTTP status code
 */
int ttc_discord_pardon_member(ttc_discord_ctx_t *ctx, uint64_t uid, 
		uint64_t gid, char *reason);

/** @brief create a guild ban.
 * @param ctx is a pointer to the discord context
 * @param uid is the user id of the account to ban
 * @param gid is the guild id to be banned from
 * @param reason is the reason if there is one. OPTIONAL
 * @param seconds is the number of seconds to delete of old messages 0-604800
 * 
 * returns a HTTP status code
 */
int ttc_discord_ban_member(ttc_discord_ctx_t *ctx, uint64_t uid, 
		uint64_t gid, char *reason, uint32_t seconds);

/**
 * @brief disable communication of a member until a timestamp
 * @param ctx is a pointer ot the discord context
 * @param uid is the user id of the account whose communication should be disabled
 * @param gid is the guild id that should be used
 * @param end_timestamp is ISO8601 timestamp until when the communication should be disabled.
 *				It can only be up to 28 days in the future
 * @param reason to show in the audit log. Can be NULL
 *	
 * @return http status code of the request
 */
int ttc_discord_timeout_member(ttc_discord_ctx_t *ctx, uint64_t uid,
		uint64_t gid, const char *end_timestamp, const char *reason);

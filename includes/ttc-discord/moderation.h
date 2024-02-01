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
	

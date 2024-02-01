#include <stdint.h>
#include <stdbool.h>

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

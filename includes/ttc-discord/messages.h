#include <ttc-discord/discord.h>
#include <stdint.h>
int ttc_discord_send_embed(ttc_discord_embed_t *embed, ttc_discord_ctx_t *ctx, uint64_t cid);
int ttc_discord_send_simple_message(const char *str, ttc_discord_ctx_t *ctx, uint64_t cid);
	

#pragma once
#include <ttc-discord/discord.h>
#include <ttc-discord/interaction.h>

void echo_handle(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url);
void userinfo_handle(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
										 const char *url);
void kick_handle(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url);
void pardon_handle(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url);
void ban_handle(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url);
void timeout_handle(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
										const char *url);

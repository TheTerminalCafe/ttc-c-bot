#pragma once
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>

void ttc_embed_modal_submit(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
														const char *url);

#pragma once
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>

int ttc_discord_create_select_menu(ttc_discord_ctx_t *ctx, uint32_t type, const char *menu_id,
																	 uint64_t channel, uint32_t max);
int ttc_discord_add_component_listener(ttc_discord_ctx_t *ctx, const char *modal_id,
																			 void (*callback)(ttc_discord_interaction_t *,
																												ttc_discord_ctx_t *, const char *));
int ttc_discord_create_button(ttc_discord_ctx_t *ctx, const char *btn_id, int btn_style,
															const char *text, uint64_t channel);

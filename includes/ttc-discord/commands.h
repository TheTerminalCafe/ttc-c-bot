#pragma once
#include <discord.h>


int ttc_discord_add_modal_listener(ttc_discord_ctx_t *ctx, const char *modal_id, void (*callback)(ttc_discord_interaction_t *, ttc_discord_ctx_t *, const char *));	
int discord_create_application_command(command_t *command, ttc_discord_ctx_t *ctx, 
		void (*callback)(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url));	


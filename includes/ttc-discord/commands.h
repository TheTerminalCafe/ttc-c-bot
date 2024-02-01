#include <discord.h>

int discord_create_application_command(command_t *command, ttc_discord_ctx_t *ctx, 
		void (*callback)(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url));	


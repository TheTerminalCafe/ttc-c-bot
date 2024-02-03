#include "ttc-discord/gateway.h"
#include <discord.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/interaction.h>

int ttc_discord_add_modal_listener(ttc_discord_ctx_t *ctx, const char *modal_id,
																	 void (*callback)(ttc_discord_interaction_t *,
																										ttc_discord_ctx_t *, const char *)) {
	cmd_listeners_t listener, *tmp;
	printf("%lu\n", ctx->modals);
	tmp = realloc(ctx->modal_callbacks, (ctx->modals + 1) * sizeof(cmd_listeners_t));

	listener.cmd_callback = callback;
	listener.name = modal_id;

	tmp[ctx->modals] = listener;

	ctx->modal_callbacks = tmp;
	ctx->modals++;

	return 0;
}

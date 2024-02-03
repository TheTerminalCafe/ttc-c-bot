#include "ttc-discord/gateway.h"
#include <ttc-discord/api.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/interaction.h>

int ttc_bot_edit_echo(ttc_discord_interaction_t *interaction, snowflake_t cid, snowflake_t mid) {
	const char *fmt = "/channels/%lu/messages/%lu";
	char *message_url;
}

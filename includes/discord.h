#pragma once

#include <openssl/ssl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-http.h>
#include <ttc-ws.h>

typedef struct cmd_listeners cmd_listeners_t;

struct cmd_listeners {
	const char *name; /**Command Name to call this command on*/
	void (*cmd_callback)(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx,
											 const char *url);
};

struct _ttc_discord_ctx_s {
	SSL_CTX *ssl_ctx;
	ttc_wss_t *gateway;
	SSL *api;

	char *token;
	char *api_token;
	char *app_id;
	char *resume_url;
	char *session_id;
	char *gateway_url;
	uint64_t sequence;
	pthread_t read_thread, heart_thread;
	int running;

	cmd_listeners_t *command_callbacks;
	uint64_t callbacks;

	cmd_listeners_t *modal_callbacks;
	uint64_t modals;

	cmd_listeners_t *components_callbacks;
	uint64_t components;

	uint64_t heart_interval;
};

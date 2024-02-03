#pragma once

typedef struct _ttc_discord_ctx_s ttc_discord_ctx_t;

#include <stdbool.h>
#include <stdint.h>

typedef uint64_t snowflake_t;

typedef struct ttc_discord_embed_field_s {
	char *name;
	char *value;
	bool inln;
} ttc_discord_embed_field_t;

typedef struct ttc_discord_embed_s {
	char *title, *type, *description, *url;
	/*Todo: Timestamp*/
	uint32_t color;

	uint32_t field_count;
	ttc_discord_embed_field_t fields[25];
} ttc_discord_embed_t;

typedef struct {
	char *label;
	uint32_t type;
	uint32_t style;
	char *value;
	char *id;
	bool required;
} ttc_modal_field_t;

typedef struct {
	char *name;
	char *id;
	ttc_modal_field_t fields[5]; /*TODO: Support for more complex modals*/
	uint32_t field_count;
} ttc_discord_modal_t;

typedef struct command_choices {
	char *name;

	/*TODO: locales*/

	union {
		char *string;
		int integer;
		double floatint;
	} value;
} command_choices_t;

typedef struct command_opt {
	int required;
	char *name;
	char *description;
	command_choices_t *choices;
	int type;
} command_opt_t;

typedef struct command {
	char *name;
	int type;
	char *description;
	command_opt_t *options;
	int option_count;
	bool allow_in_dms;
	uint64_t default_permissions;
} command_t;

int ttc_discord_run(ttc_discord_ctx_t *ctx);
void ttc_discord_ctx_destroy(ttc_discord_ctx_t *ctx);
ttc_discord_ctx_t *ttc_discord_ctx_create(char *token);

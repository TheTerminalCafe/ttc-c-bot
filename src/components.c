#include <json-c/json_object.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ttc-discord/discord.h>
#include <ttc-discord/gateway.h>
#include <ttc-discord/api.h>
#include <ttc-discord/interaction.h>

#include <ttc-http.h>
#include <ttc-log.h>

#include <json-c/json.h>



void ttc_self_roles_picked(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url) {
	ttc_discord_component_data_t *component = interaction->data.component;
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	ttc_discord_member_t *member = interaction->member;
	char *role_url;
	int length, res;
	bool found = 0;
	/*Responsed with loading for now*/
	ttc_discord_interaction_loading(ctx, url);

	for(size_t ind = 0; ind < component->count; ind++) {
		for(size_t jnd = 0; jnd < member->role_count; jnd++) {
			/*If a user takes a role they alread have remove it.
			 * if they take a role they don't have add it
			 */
			if(strcmp(component->values[ind], member->roles[jnd]) == 0) {
				/*Role exists already*/
				res = discord_user_role(ctx, interaction->guild_id, interaction->member->user.id,
						strtoull(component->values[ind], NULL, 10), TTC_HTTP_METHOD_DELETE);
				if(res != 204) {
					goto out;
				}
				found = 1;
				
				break;
			}
		}
		if(!found) {
			res = discord_user_role(ctx, interaction->guild_id, interaction->member->user.id,
						strtoull(component->values[ind], NULL, 10), TTC_HTTP_METHOD_PUT);
			if(res != 204) break;
		}
	}
	out:

	if(res == 204) {
		ttc_discord_interaction_loading_respond(ctx, "success", "Your roles are updated", 0x00ff00, interaction);
	} else {
		ttc_discord_interaction_loading_respond(ctx, "Failed to update roles", "Unable to update roles", 0xff0000, interaction);	
	}
	

}


void ttc_ticket_modal_create(ttc_discord_interaction_t *interaction, ttc_discord_ctx_t *ctx, const char *url) {
	json_object *id, *title, *components, *row, *submit,
				*title_input, *sysinfo, *description,
				*row_type, *title_type, *title_style,
				*sysinfo_type, *sysinfo_style,
				*desc_type, *desc_style, *title_id,
				*label, *modal_id, *type, *response,
				*catergory, *modal, *row_components;
	char *length_str;
	ttc_http_request_t *request;
	ttc_http_response_t *http_response;

	response = json_object_new_object();
	type = json_object_new_int(9);

	modal = json_object_new_object();
	modal_id = json_object_new_string("ticket_modal");
	json_object_object_add(modal, "custom_id", modal_id);
	components = json_object_new_array();
	json_object_object_add(modal, "components", components);

	row_components = json_object_new_array();
	id = json_object_new_string("ticket_submit");
	row = json_object_new_object();
	row_type = json_object_new_int(DiscordComponentActionRow);
	

	json_object_object_add(row, "type", row_type);
	json_object_object_add(row, "components", row_components);
	json_object_array_add(components, row);

	title_input = json_object_new_object();
	title = json_object_new_string("Create Ticket:");
	json_object_object_add(modal, "title", title);
	label = json_object_new_string("Ticket Title:");
	title_type = json_object_new_int(DiscordComponentTextInput);
	title_style = json_object_new_int(DiscordTextInputSingleLine);
	title_id = json_object_new_string("ticket_title");

	json_object_object_add(title_input, "type", title_type);
	json_object_object_add(title_input, "style", title_style);
	json_object_object_add(title_input, "label", label);
	json_object_object_add(title_input, "custom_id", title_id);
	

	json_object_array_add(row_components, title_input);
	json_object_object_add(response, "type", type);
	json_object_object_add(response, "data", modal);




	int length = snprintf(NULL, 0, "%lu", strlen(json_object_to_json_string(response)));
	length_str = calloc(1, length + 1);
	snprintf(length_str, length + 1, "%lu", strlen(json_object_to_json_string(response)));

	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_POST);
	
	http_response = ttc_discord_api_send_json(ctx, request, response);
	TTC_LOG_INFO("Sending: %s\n", ttc_http_request_get_str(request));
	

	json_object_put(response);
	ttc_http_response_free(http_response);
	ttc_http_request_free(request);
	free(length_str);
}

#include <discord.h>

#include <json-c/json_object.h>
#include <ttc-log.h>
#include <ttc-http.h>
#include <ttc-discord/api.h>


json_object *discord_get_guild_member(ttc_discord_ctx_t *ctx, uint64_t gid, uint64_t uid) {
	ttc_http_request_t *get_member;
	ttc_http_response_t *response;
	const char *fmt = "/api/v10/guilds/%lu/members/%lu";
	char *buffer;
	int length;
	
	length = snprintf(NULL, 0, fmt, gid, uid);
	buffer = calloc(1, length + 1);
	length = snprintf(buffer, length + 1, fmt, gid, uid);

	get_member = ttc_http_new_request();
	
	ttc_http_request_set_http_version(get_member, HTTP_VER_11);
	ttc_http_request_set_path(get_member, buffer);
	ttc_http_request_set_method(get_member, TTC_HTTP_METHOD_GET);
	
	response = ttc_discord_api_send_request(ctx, get_member);
	if(response->status != 200) {
		return NULL;	
	}

	json_object *json = json_tokener_parse(response->data);
	free(buffer);
	ttc_http_response_free(response);
	ttc_http_request_free(get_member);
	return json;
}

int discord_get_user_position(ttc_discord_ctx_t *ctx, uint64_t gid, uint64_t uid) {
	ttc_http_request_t *get_roles;
	json_object *member, *mroles;
	json_object *roles;
	char *fmt = "/api/v10/guilds/%lu/roles";
	int length;
	uint32_t highest = 0;
	
	member = discord_get_guild_member(ctx, gid, uid);
	if(!member) {
		return 0;
	}

	json_object_object_get_ex(member, "roles", &mroles);
	if(!mroles) {
		json_object_put(member);	
		return 0;
	}
	
	get_roles = ttc_http_new_request();

	length = snprintf(NULL, 0, fmt, gid);
	
	char *buffer = calloc(1, length + 1);
	length = snprintf(buffer, length + 1, fmt, gid);
	
	ttc_http_request_set_http_version(get_roles, HTTP_VER_11);
	ttc_http_request_set_path(get_roles, buffer);
	ttc_http_request_set_method(get_roles, TTC_HTTP_METHOD_GET);
	TTC_LOG_DEBUG("%s\n", ttc_http_request_get_str(get_roles));

	ttc_http_response_t *response = ttc_discord_api_send_request(ctx, get_roles);
	TTC_LOG_DEBUG("%lu guild roles\n%s\n%s\n", gid, response->headers, response->data);
	roles = json_tokener_parse(response->data);
	
	ttc_http_response_free(response);

	for(size_t index = 0; index < json_object_array_length(mroles); index++) {
		json_object *mrole_id = json_object_array_get_idx(mroles, index);

		for(size_t jindex = 0; jindex < json_object_array_length(roles); jindex++) {
			json_object *role, *id, *pos; 
			role = json_object_array_get_idx(roles, jindex);
			json_object_object_get_ex(role, "id", &id);
			if(strcmp(json_object_get_string(id), json_object_get_string(mrole_id)) == 0) {
				json_object_object_get_ex(role, "position", &pos);
				if(highest < json_object_get_int64(pos)) {
					highest = json_object_get_int64(pos);
				}
			}
		}
	}

	free(buffer);
	json_object_put(member);
	json_object_put(roles);
	ttc_http_request_free(get_roles);
	ttc_http_response_free(response);
	return highest;
}


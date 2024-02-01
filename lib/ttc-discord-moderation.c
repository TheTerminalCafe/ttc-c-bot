#include <discord.h>
#include <ttc-discord/api.h>

#include <ttc-log.h>

int ttc_discord_kick_member(ttc_discord_ctx_t *ctx, uint64_t uid, 
		uint64_t gid, char *reason) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	ttc_discord_embed_t embed;
	int result = 0;
	char *url;
	const char *fmt = "/api/v10/guilds/%lu/members/%lu";
	int length = 0;

	length = snprintf(NULL, 0, fmt, gid, uid);
	
	url = calloc(1, length + 1);

	snprintf(url, length + 1, fmt, gid, uid);
	
	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_DELETE);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	if(reason) {
		ttc_http_request_add_header(request, "X-Audit-Log-Reason", reason);
	}

	response = ttc_discord_api_send_request(ctx, request);
	
	result = response->status;
	
	ttc_http_response_free(response);
	ttc_http_request_free(request);
	free(url);
	return result;
}

int ttc_discord_pardon_member(ttc_discord_ctx_t *ctx, uint64_t uid, 
		uint64_t gid, char *reason) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	ttc_discord_embed_t embed;
	int result = 0;
	char *url;
	const char *fmt = "/api/v10/guilds/%lu/bans/%lu";
	int length = 0;

	length = snprintf(NULL, 0, fmt, gid, uid);
	
	url = calloc(1, length + 1);

	snprintf(url, length + 1, fmt, gid, uid);
	
	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_DELETE);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	if(reason) {
		ttc_http_request_add_header(request, "X-Audit-Log-Reason", reason);
	}

	response = ttc_discord_api_send_request(ctx, request);
	
	result = response->status;
	
	ttc_http_response_free(response);
	ttc_http_request_free(request);
	free(url);
	return result;
}



int ttc_discord_ban_member(ttc_discord_ctx_t *ctx, uint64_t uid, 
		uint64_t gid, char *reason, uint32_t seconds) {
	ttc_http_request_t *request;
	ttc_http_response_t *response;
	json_object *ban_json, *sec_to_del;
	ttc_discord_embed_t embed;
	int result = 0;
	char *url;
	const char *fmt = "/api/v10/guilds/%lu/bans/%lu";
	int length = 0;

	length = snprintf(NULL, 0, fmt, gid, uid);
	
	url = calloc(1, length + 1);

	ban_json = json_object_new_object();
	sec_to_del = json_object_new_int(seconds);
	json_object_object_add(ban_json, "delete_message_seconds", sec_to_del);

	snprintf(url, length + 1, fmt, gid, uid);
	
	request = ttc_http_new_request();
	ttc_http_request_set_path(request, url);
	ttc_http_request_set_method(request, TTC_HTTP_METHOD_PUT);
	ttc_http_request_set_http_version(request, HTTP_VER_11);
	if(reason) {
		ttc_http_request_add_header(request, "X-Audit-Log-Reason", reason);
	}

	response = ttc_discord_api_send_json(ctx, request, ban_json);
	
	result = response->status;
	
	TTC_LOG_DEBUG("Ban response Data: %s\n", response->data);
	TTC_LOG_DEBUG("Ban Response Headers: %s\n", response->headers);


	ttc_http_response_free(response);
	ttc_http_request_free(request);
	free(url);
	return result;
}

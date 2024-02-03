#pragma once

#include <stdint.h>

typedef enum ttc_log_levels {
	TTC_LOG_INFO,
	TTC_LOG_DEBUG,
	TTC_LOG_WARN,
	TTC_LOG_ERROR,
	TTC_LOG_FATAL,
	TTC_LOG_MAX,
} ttc_log_levels_t;

int ttc_log(ttc_log_levels_t level, uint64_t line, const char *file, const char *fmt, ...);
void ttc_log_set_level(ttc_log_levels_t level);
int ttc_log_printf(ttc_log_levels_t level, const char *fmt, ...);

#define ttc_info(...) ttc_log(TTC_LOG_INFO, __LINE__, __FILE__, __VA_ARGS__)
#define ttc_debug(...) ttc_log(TTC_LOG_DEBUG, __LINE__, __FILE__, __VA_ARGS__)
#define ttc_warn(...) ttc_log(TTC_LOG_WARN, __LINE__, __FILE__, __VA_ARGS__)
#define ttc_error(...) ttc_log(TTC_LOG_ERROR, __LINE__, __FILE__, __VA_ARGS__)
#define ttc_fatal(...) ttc_log(TTC_LOG_FATAL, __LINE__, __FILE__, __VA_ARGS__)

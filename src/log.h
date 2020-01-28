#ifndef LOG_H
#define LOG_H

enum log_level_t {
	DEBUG,
	INFO,
	ERROR,
};


void log_msg(enum log_level_t level, char *format, ...);

#define LOG(level, format, ...) log_msg(level, format, ##__VA_ARGS__)
#define LOG_DEBUG(format, ...) log_msg(DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) log_msg(INFO, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) log_msg(ERROR, format, ##__VA_ARGS__)

#endif

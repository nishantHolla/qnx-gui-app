#ifndef RMP_LOG_H_
#define RMP_LOG_H_

void rmp_log_error(const char* author, const char* fmt, ...);
void rmp_log_info(const char* author, const char* fmt, ...);
void rmp_log_warn(const char* author, const char* fmt, ...);

#endif // !RMP_LOG_H_

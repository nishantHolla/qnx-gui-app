#include "rmp_log.h"

#include <stdio.h>
#include <stdarg.h>

static void rmp_log_base(FILE* stream, const char* level, const char* author,
                         const char* fmt, va_list args);

void rmp_log_error(const char* author, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  rmp_log_base(stderr, "ERRR", author, fmt, args);

  va_end(args);
}

void rmp_log_info(const char* author, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  rmp_log_base(stdout, "INFO", author, fmt, args);

  va_end(args);
}

void rmp_log_warn(const char* author, const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  rmp_log_base(stdout, "WARN", author, fmt, args);

  va_end(args);
}

static void rmp_log_base(FILE* stream, const char* level, const char* author,
                         const char* fmt, va_list args) {
  fprintf(stream, "[%s] %s: ", level, author);
  vfprintf(stream, fmt, args);
}

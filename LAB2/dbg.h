#ifndef __DBG_H_
#define __DBG_H_

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef NDEBUG
#define DEBUG(M, ...)
#else
#define DEBUG(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define CLEAN_ERRNO() (errno == 0 ? "None" : strerror(errno))

#define LOG_ERR(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s)" M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define LOG_WARN(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s)" M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)
#define LOG_INFO(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define CHECK(A, M, ...) if (!(A)) { log_err(M, ##__VA_ARGS__); errno = 0; goto error; }
#define SENTINEL(M, ...) { log_err(M, ##__VA_ARGS__); errno = 0; goto error; }

#define CHECK_MEM(A) check((A), "Out of memory.")
#define CHECK_DEBUG(A, M, ...) if((!A)) { debug(M, ##__VA_ARGS__); errno = 0; goto error; }
#endif

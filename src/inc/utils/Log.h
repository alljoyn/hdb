#ifndef _LOG_H
#define _LOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_FILE_PATH	"/tmp/bridge.log"

#ifdef _DEBUG
#if 0
	#define LOGD(tag, fmt,...) printf("[%s][%s: %d] "fmt"\n", tag, __func__, __LINE__, ##__VA_ARGS__)
	#define ZBLOGD(tag, id, fmt,...) printf("[%s-%d][%s: %d] "fmt"\n", tag, id, __func__, __LINE__, ##__VA_ARGS__)
	#define VDLOGD(tag, id, fmt,...) printf("[%s-%d][%s: %d] "fmt"\n", tag, id, __func__, __LINE__, ##__VA_ARGS__)
#else
	void WriteLog(const char* str, ...);
	char* GetCurrentTimeStr();
	#define LOGD(tag, fmt,...) WriteLog("[%s][%s][%s: %d] "fmt"\n", GetCurrentTimeStr(), tag, __func__, __LINE__, ##__VA_ARGS__)
	#define ZBLOGD(tag, id, fmt,...) WriteLog("[%s][%s-%d][%s: %d] "fmt"\n", GetCurrentTimeStr(), tag, id, __func__, __LINE__, ##__VA_ARGS__)
	#define VDLOGD(tag, id, fmt,...) WriteLog("[%s][%s-%d][%s: %d] "fmt"\n", GetCurrentTimeStr(), tag, id, __func__, __LINE__, ##__VA_ARGS__)
#endif // #if 0
#else
	#define LOGD
	#define ZBLOGD
	#define VDLOGD
#endif // #ifdef _DEBUG


#define IF_NULL_RETURN(x) \
						do { \
							if (x==NULL) { \
								LOGD(TAG, "Return by null."); \
								return; \
							} \
						} while (0)

#define IF_NULL_RETURN_FAIL(x) \
						do { \
							if (x==NULL) { \
								LOGD(TAG, "Return by null."); \
								return ER_FAIL; \
							} \
						} while (0)



#endif // _LOG_H

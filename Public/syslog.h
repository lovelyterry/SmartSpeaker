#ifndef __SYSLOG_H
#define __SYSLOG_H

#include <stdint.h>

#define __DEBUG_MODE
#define __RECOED_LEVEL  7
/*
emerg	紧急情况，系统不可用（例如系统崩溃），一般会通知所有用户。
alert	需要立即修复，例如系统数据库损坏。
crit	危险情况，例如硬盘错误，可能会阻碍程序的部分功能。
err		一般错误消息。
warning	警告。
notice	不是错误，但是可能需要处理。
info	通用性消息，一般用来提供有用信息。
debug	调试程序产生的信息。
*/
#define LOG_EMERG   0   /* system is unusable */
#define LOG_ALERT   1   /* action must be taken immediately */
#define LOG_CRIT    2   /* critical conditions */
#define LOG_ERR     3   /* error conditions */
#define LOG_WARNING 4   /* warning conditions */
#define LOG_NOTICE  5   /* normal but significant condition */
#define LOG_INFO    6   /* informational */
#define LOG_DEBUG   7   /* debug-level messages */


#pragma __printf_args
extern int ts_printf(const char * __restrict /*format*/, ...) __attribute__((__nonnull__(1)));

#pragma __printf_args
extern int log_record(const char * __restrict /*format*/, ...) __attribute__((__nonnull__(1)));

#ifdef __DEBUG_MODE
#define __log_record(result, expect, level, fmt, arg...) \
do{\
	volatile int r_temp = (result);\
	volatile int e_temp = (expect);\
	\
	if(e_temp != r_temp && (level) <= __RECOED_LEVEL )\
	{\
		log_record("\r\nlevel: %d\r\nfile: %s\r\nline: %d\r\nfunc: %s\r\nresult: %d\r\nexpect: %d\r\ninfo: "fmt"\r\n\r\n", \
					level, __FILE__, __LINE__, __FUNCTION__, r_temp, e_temp, ##arg);\
	}\
	else\
	{}\
} while(0)
#else
#define __log_record(...) ((void)0)
#endif


#endif

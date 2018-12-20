/*
 * Copyright (C) 2015-2017 Alibaba Group Holding Limited
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "log_impl.h"
#include "aos/kernel.h"
#include "aos/log.h"
#ifdef CONFIG_AOS_CLI
#include <aos/cli.h>
#endif // CONFIG_AOS_CLI

unsigned int aos_log_level = AOS_LL_V_DEBUG | AOS_LL_V_INFO | AOS_LL_V_WARN |
                             AOS_LL_V_ERROR | AOS_LL_V_FATAL;
static aos_mutex_t log_mutex;

int log_get_mutex()
{
    return 0 == aos_mutex_lock(&log_mutex, AOS_WAIT_FOREVER);
}

void log_release_mutex()
{
    aos_mutex_unlock(&log_mutex);
}

#ifndef csp_printf
#ifdef _WIN32
// definition will allocate in hw.c
extern int csp_printf(const char *fmt, ...);
#else
__attribute__((weak)) int csp_printf(const char *fmt, ...)
{
    va_list args;
    int     ret;


    va_start(args, fmt);
    ret = vprintf(fmt, args);
    va_end(args);

    fflush(stdout);


    return ret;
}
#endif
#endif

void aos_set_log_level(aos_log_level_t log_level)
{
    unsigned int value = 0;

    switch (log_level) {
        case AOS_LL_NONE:
            value |= AOS_LL_V_NONE;
            break;
        case AOS_LL_DEBUG:
            value |= AOS_LL_V_DEBUG;
        case AOS_LL_INFO:
            value |= AOS_LL_V_INFO;
        case AOS_LL_WARN:
            value |= AOS_LL_V_WARN;
        case AOS_LL_ERROR:
            value |= AOS_LL_V_ERROR;
        case AOS_LL_FATAL:
            value |= AOS_LL_V_FATAL;
            break;
        default:
            break;
    }

    aos_log_level = value;
}

#ifdef CONFIG_AOS_CLI
static void log_cmd(char *buf, int len, int argc, char **argv)
{
    const char *lvls[] = {
        [AOS_LL_FATAL] = "fatal", [AOS_LL_ERROR] = "error",
        [AOS_LL_WARN] = "warn",   [AOS_LL_INFO] = "info",
        [AOS_LL_DEBUG] = "debug",
    };

    if (argc < 2) {
        aos_cli_printf("log level : %02x\r\n", aos_get_log_level());
        return;
    }

    int i;
    for (i = 0; i < sizeof(lvls) / sizeof(lvls[0]); i++) {
        if (strncmp(lvls[i], argv[1], strlen(lvls[i]) + 1) != 0) {
            continue;
        }

        aos_set_log_level((aos_log_level_t)i);
        aos_cli_printf("set log level success\r\n");
        return;
    }
    aos_cli_printf("set log level fail\r\n");
}


struct cli_command log_cli_cmd[] = { { "loglevel", "set log level", log_cmd } };

/* log init with cli */
void log_cli_init(void)
{
    aos_log_level = AOS_LL_V_DEBUG | AOS_LL_V_INFO | AOS_LL_V_WARN |
                    AOS_LL_V_ERROR | AOS_LL_V_FATAL;
    aos_cli_register_commands(&log_cli_cmd[0],
                              sizeof(log_cli_cmd) / sizeof(struct cli_command));
    aos_mutex_new(&log_mutex);
}
#endif // CONFIG_AOS_CLI

/* log init without cli */
void log_no_cli_init(void)
{
    aos_mutex_new(&log_mutex);
}
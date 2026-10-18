/*
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "utils/logger.h"

#include <psp2/kernel/clib.h>
#include <psp2/kernel/threadmgr.h>

#include <stdbool.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#include <psp2/io/stat.h>

#define COLOR_RED    "\x1B[38;5;196m"
#define COLOR_PINK   "\x1B[38;5;212m"
#define COLOR_ORANGE "\x1B[38;5;202m"
#define COLOR_BLUE   "\x1B[38;5;32m"
#define COLOR_GREEN  "\x1B[32m"
#define COLOR_CYAN   "\x1B[36m"

#define COLOR_END    "\033[0m"

static SceKernelLwMutexWork _log_mutex;
static atomic_bool _log_mutex_ready = ATOMIC_VAR_INIT(false);

static unsigned int s_log_seq = 1;
static char s_prev_msg[1536] = {0};
static int s_prev_type = -1;
static unsigned int s_repeat_count = 0;
static bool s_log_cleared = false;

// Buffer used to format the final log line.
static char buffer_b[2048];

static unsigned int _get_next_seq(void) {
    unsigned int seq = s_log_seq;
    s_log_seq++;
    if (s_log_seq > 999) {
        s_log_seq = 1;
    }
    return seq;
}

static const char * _get_type_prefix(int t) {
    switch (t) {
        case LT_DEBUG:   return " " COLOR_PINK   "• debug"   COLOR_END "    ";
        case LT_INFO:    return " " COLOR_BLUE   "ℹ info"    COLOR_END "     ";
        case LT_WARN:    return " " COLOR_ORANGE "⚠ warning" COLOR_END "  ";
        case LT_ERROR:   return " " COLOR_RED    "⨯ error"   COLOR_END "    ";
        case LT_FATAL:   return " " COLOR_RED    "! fatal"   COLOR_END "    ";
        case LT_SUCCESS: return " " COLOR_GREEN  "! success" COLOR_END "  ";
        case LT_WAIT:    return " " COLOR_CYAN   "… waiting" COLOR_END "  ";
        default:         return " ";
    }
}

static char s_log_filepath[256] = {0};
static bool s_log_inited = false;

static void _init_log_filename(void) {
    if (s_log_inited) {
        return;
    }
    s_log_inited = true;

    sceIoMkdir(DATA_PATH "logs", 0777);

    for (unsigned int i = 1; i <= 999; i++) {
        char test_path[256];
        sceClibSnprintf(test_path, sizeof(test_path), DATA_PATH "logs/log_%03u.log", i);

        SceIoStat stat;
        if (sceIoGetstat(test_path, &stat) < 0) {
            strncpy(s_log_filepath, test_path, sizeof(s_log_filepath) - 1);
            s_log_filepath[sizeof(s_log_filepath) - 1] = '\0';
            return;
        }
    }

    sceClibSnprintf(s_log_filepath, sizeof(s_log_filepath), DATA_PATH "logs/log_001.log");
}

// Pending file output. Writing to ux0 (fputs+fflush) on the game thread cost
// up to ~50ms per flush: log_018 showed a 70-80ms spike in every 60-frame
// perf window. Lines are queued under _log_mutex and written by a low
// priority thread every 100ms; errors/fatals are still written immediately.
static char s_ring[64 * 1024];
static size_t s_ring_len = 0;
static bool s_ring_overflow = false;
static SceKernelLwMutexWork _log_io_mutex;

static void _log_drain(void) {
    static char out[sizeof(s_ring)];
    static FILE *f = NULL, *f_latest = NULL;

    sceKernelLockLwMutex(&_log_io_mutex, 1, NULL);
    sceKernelLockLwMutex(&_log_mutex, 1, NULL);
    size_t len = s_ring_len;
    memcpy(out, s_ring, len);
    s_ring_len = 0;
    s_ring_overflow = false;
    sceKernelUnlockLwMutex(&_log_mutex, 1);

    if (len > 0) {
        if (!s_log_cleared) {
            _init_log_filename();
            f = fopen(s_log_filepath, "w");
            // Also mirror to log.log for tools/scripts
            f_latest = fopen(DATA_PATH "logs/log.log", "w");
            s_log_cleared = true;
        }
        if (f) {
            fwrite(out, 1, len, f);
            fflush(f);
        }
        if (f_latest) {
            fwrite(out, 1, len, f_latest);
            fflush(f_latest);
        }
    }
    sceKernelUnlockLwMutex(&_log_io_mutex, 1);
}

static int _log_writer_thread(SceSize args, void *argp) {
    (void) args; (void) argp;
    for (;;) {
        sceKernelDelayThread(100 * 1000);
        _log_drain();
    }
    return 0;
}

static void _log_flush_internal(void) {
    if (s_repeat_count == 0) {
        return;
    }

    unsigned int seq = _get_next_seq();
    const char *prefix = _get_type_prefix(s_prev_type);

    if (s_repeat_count > 1) {
        sceClibSnprintf(buffer_b, sizeof(buffer_b), "[%03u]%s%s (x%u)\n",
                        seq, prefix, s_prev_msg, s_repeat_count);
    } else {
        sceClibSnprintf(buffer_b, sizeof(buffer_b), "[%03u]%s%s\n",
                        seq, prefix, s_prev_msg);
    }

    sceClibPrintf("%s", buffer_b);

    // Only queue here: the file write happens on log_writer (see _log_drain).
    size_t len = strlen(buffer_b);
    if (s_ring_len + len > sizeof(s_ring))
        s_ring_overflow = true;  // writer fell behind: drained synchronously by the caller
    else {
        memcpy(s_ring + s_ring_len, buffer_b, len);
        s_ring_len += len;
    }

    s_repeat_count = 0;
    s_prev_type = -1;
    s_prev_msg[0] = '\0';
}

void _log_print(int t, const char* fmt, ...) {
    if (t < LT_DEBUG || t > LT_WAIT) {
        return;
    }

    if (!atomic_load_explicit(&_log_mutex_ready, memory_order_relaxed)) {
        int ret = sceKernelCreateLwMutex(&_log_mutex, "log_lock", 0, 0, NULL);
        if (ret < 0) {
            sceClibPrintf("Error: failed to create log mutex: 0x%x\n", ret);
            return;
        }
        sceKernelCreateLwMutex(&_log_io_mutex, "log_io_lock", 0, 0, NULL);
        atomic_store_explicit(&_log_mutex_ready, true, memory_order_relaxed);
        // Lowest priority, off the game (core 0) and audio mixer (core 1) cores.
        SceUID th = sceKernelCreateThread("log_writer", _log_writer_thread, 0xBF, 0x4000, 0,
                                          SCE_KERNEL_CPU_MASK_USER_2, NULL);
        if (th >= 0)
            sceKernelStartThread(th, 0, NULL);
    }
    sceKernelLockLwMutex(&_log_mutex, 1, NULL);

    char current_msg[1536];
    va_list list;
    va_start(list, fmt);
    sceClibVsnprintf(current_msg, sizeof(current_msg), fmt, list);
    va_end(list);

    // Strip trailing newlines or carriage returns
    size_t len = strlen(current_msg);
    while (len > 0 && (current_msg[len - 1] == '\n' || current_msg[len - 1] == '\r')) {
        current_msg[--len] = '\0';
    }

    if (s_repeat_count > 0 && s_prev_type == t && strcmp(current_msg, s_prev_msg) == 0) {
        s_repeat_count++;
    } else {
        if (s_repeat_count > 0) {
            _log_flush_internal();
        }
        s_prev_type = t;
        strncpy(s_prev_msg, current_msg, sizeof(s_prev_msg) - 1);
        s_prev_msg[sizeof(s_prev_msg) - 1] = '\0';
        s_repeat_count = 1;

        if (t == LT_ERROR || t == LT_FATAL) {
            _log_flush_internal();
        }
    }

    bool drain_now = t == LT_ERROR || t == LT_FATAL || s_ring_overflow;
    if (atomic_load_explicit(&_log_mutex_ready, memory_order_relaxed)) {
        sceKernelUnlockLwMutex(&_log_mutex, 1);
    }
    if (drain_now)
        _log_drain();  // crash triage needs errors on disk before the crash
}

void log_flush(void) {
    if (!atomic_load_explicit(&_log_mutex_ready, memory_order_relaxed)) {
        return;
    }
    sceKernelLockLwMutex(&_log_mutex, 1, NULL);
    if (s_repeat_count > 0) {
        _log_flush_internal();
    }
    sceKernelUnlockLwMutex(&_log_mutex, 1);
}

/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "reimpl/io.h"

#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdarg.h>
#include <psp2/kernel/threadmgr.h>

#ifdef USE_SCELIBC_IO
#include <libc_bridge/libc_bridge.h>
#endif

#include "utils/logger.h"
#include "utils/utils.h"

// Includes the following inline utilities:
// int oflags_musl_to_newlib(int flags);
// dirent64_bionic * dirent_newlib_to_bionic(struct dirent* dirent_newlib);
// void stat_newlib_to_bionic(struct stat * src, stat64_bionic * dst);
#include "reimpl/bits/_struct_converters.c"

// Estructura de datos en Vita (ux0_data/gtracingmotoracademyhd/):
//   GTRacing/  <- /sdcard/gameloft/games/GTRacing/<file> (AI.bin, *.bar, *.swf, ...)
//   saves/     <- /data/data/com.gameloft.android.TBFV.GloftGTHP.ML/... (dir privado)
//   res/, assets/ <- servidos por los stubs GLResLoader (generated_jni_stubs.c)
//   logs/      <- logs del loader (vita_logs_dir)
// Solo se redirige el prefijo; el resto de la ruta se conserva tal cual.
#define GAMELOFT_SDCARD_PREFIX "/sdcard/gameloft/games/GTRacing/"
#define GAMELOFT_SDCARD_BARE "/sdcard/gameloft/games/GTRacing"
#define GAMELOFT_DATA_PREFIX "/data/data/com.gameloft.android.TBFV.GloftGTHP.ML"

// Rewrites an absolute Android path to its real location under DATA_PATH.
// Returns true and fills `out` on match (out untouched otherwise, caller falls
// through to `path` unchanged). Tolerates "//" after the sdcard prefix because
// the Java side builds paths as ".../GTRacing//" + name.
static int remap_android_path(const char * path, char * out, size_t out_size) {
    size_t plen = strlen(GAMELOFT_SDCARD_PREFIX);
    if (strncmp(path, GAMELOFT_SDCARD_PREFIX, plen) == 0) {
        const char * rest = path + plen;
        while (*rest == '/') rest++; // colapsa "//" de Java
        snprintf(out, out_size, DATA_PATH"GTRacing/%s", rest);
        return 1;
    }
    if (strcmp(path, GAMELOFT_SDCARD_BARE) == 0) {
        snprintf(out, out_size, DATA_PATH"GTRacing");
        return 1;
    }
    size_t dlen = strlen(GAMELOFT_DATA_PREFIX);
    if (strncmp(path, GAMELOFT_DATA_PREFIX, dlen) == 0) {
        const char * rest = path + dlen;
        while (*rest == '/') rest++;
        snprintf(out, out_size, DATA_PATH"saves/%s", rest);
        return 1;
    }
    return 0;
}

FILE * fopen_soloader(const char * filename, const char * mode) {
    if (strcmp(filename, "/proc/cpuinfo") == 0) {
        return fopen_soloader("app0:/cpuinfo", mode);
    } else if (strcmp(filename, "/proc/meminfo") == 0) {
        return fopen_soloader("app0:/meminfo", mode);
    }

    char remapped[256];
    if (remap_android_path(filename, remapped, sizeof(remapped))) {
        return fopen_soloader(remapped, mode);
    }

#ifdef USE_SCELIBC_IO
    FILE* ret = sceLibcBridge_fopen(filename, mode);
#else
    FILE* ret = fopen(filename, mode);
#endif

    if (!ret)
        l_warn("fopen(%s, %s): %p", filename, mode, ret);

    return ret;
}

int open_soloader(const char * path, int oflag, ...) {
    if (strcmp(path, "/proc/cpuinfo") == 0) {
        return open_soloader("app0:/cpuinfo", oflag);
    } else if (strcmp(path, "/proc/meminfo") == 0) {
        return open_soloader("app0:/meminfo", oflag);
    }

    char remapped[256];
    if (remap_android_path(path, remapped, sizeof(remapped))) {
        path = remapped; // cae al flujo normal y conserva mode de O_CREAT
    }

    mode_t mode = 0666;
    if (((oflag & BIONIC_O_CREAT) == BIONIC_O_CREAT) ||
        ((oflag & BIONIC_O_TMPFILE) == BIONIC_O_TMPFILE)) {
        va_list args;
        va_start(args, oflag);
        mode = (mode_t)(va_arg(args, int));
        va_end(args);
    }

    oflag = oflags_bionic_to_newlib(oflag);
    int ret = open(path, oflag, mode);
    if (ret < 0)
        l_warn("open(%s, %x): %i", path, oflag, ret);
    return ret;
}

int fstat_soloader(int fd, stat64_bionic * buf) {
    struct stat st;
    int res = fstat(fd, &st);

    if (res == 0)
        stat_newlib_to_bionic(&st, buf);

    return res;
}

int stat_soloader(const char * path, stat64_bionic * buf) {
    char remapped[256];
    if (remap_android_path(path, remapped, sizeof(remapped))) {
        return stat_soloader(remapped, buf);
    }

    struct stat st;
    int res = stat(path, &st);

    if (res == 0)
        stat_newlib_to_bionic(&st, buf);

    return res;
}

int fclose_soloader(FILE * f) {
#ifdef USE_SCELIBC_IO
    int ret = sceLibcBridge_fclose(f);
#else
    int ret = fclose(f);
#endif

    return ret;
}

int close_soloader(int fd) {
    int ret = close(fd);
    return ret;
}

DIR* opendir_soloader(char* _pathname) {
    char remapped[256];
    if (remap_android_path(_pathname, remapped, sizeof(remapped))) {
        return opendir_soloader(remapped);
    }
    DIR* ret = opendir(_pathname);
    return ret;
}

struct dirent64_bionic * readdir_soloader(DIR * dir) {
    static struct dirent64_bionic dirent_tmp;

    struct dirent* ret = readdir(dir);

    if (ret) {
        dirent_newlib_to_bionic_buf(ret, &dirent_tmp);
        return &dirent_tmp;
    }

    return NULL;
}

int readdir_r_soloader(DIR * dirp, dirent64_bionic * entry,
                       dirent64_bionic ** result) {
    struct dirent dirent_tmp;
    struct dirent * pdirent_tmp;

    int ret = readdir_r(dirp, &dirent_tmp, &pdirent_tmp);

    if (ret == 0 && pdirent_tmp != NULL) {
        dirent_newlib_to_bionic_buf(&dirent_tmp, entry);
        *result = entry;
    } else {
        *result = NULL;
    }

    return ret;
}

int closedir_soloader(DIR * dir) {
    int ret = closedir(dir);
    return ret;
}

int fcntl_soloader(int fd, int cmd, ...) {
    l_warn("fcntl(%i, %i, ...): not implemented", fd, cmd);
    return 0;
}

int ioctl_soloader(int fd, int request, ...) {
    l_warn("ioctl(%i, %i, ...): not implemented", fd, request);
    return 0;
}

int fsync_soloader(int fd) {
    int ret = fsync(fd);
    l_debug("fsync(%i): %i", fd, ret);
    return ret;
}

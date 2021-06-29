#include "ns/utils/logger.h"
#include <string.h>
#include <time.h>
#include <sys/types.h>

#define LOGGER_MAX_CALLBACKS 32
#define LOGGER_UNKNOWN_RANK -1
#define LOGGER_MAX_RANK_CHARS 4

typedef struct {
    log_LogFn fn;
    void *udata;
    int level;
} Callback;

static struct {
    void *udata;
    log_LockFn lock;
    int level;
    int rank;
    bool quiet;
    bool colors;
    Callback callbacks[LOGGER_MAX_CALLBACKS];
} L = {
        .rank = LOGGER_UNKNOWN_RANK,
        .quiet = false,
        .colors = false,
};

static const char *level_strings[] = {
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
        "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

static void stdout_callback(log_Event *ev) {
    char time_buf[64];
    char rank[LOGGER_MAX_RANK_CHARS + 1] = "\0";

    time_buf[strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    if (L.rank != LOGGER_UNKNOWN_RANK)
        snprintf(rank, LOGGER_MAX_RANK_CHARS + 1, "%d", L.rank);

    fprintf(ev->udata, "[%.*s%s] %s ",
            (LOGGER_MAX_RANK_CHARS < strlen(rank)) ? 0 : (int) (LOGGER_MAX_RANK_CHARS - strlen(rank)),
            "--------------------------------", rank, time_buf);

    if (L.colors) {
        fprintf(
                ev->udata, "%s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m ",
                level_colors[ev->level], level_strings[ev->level], ev->file, ev->line);
    } else {
        fprintf(
                ev->udata, "%-5s %s:%d: ",
                level_strings[ev->level], ev->file, ev->line);
    }
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void file_callback(log_Event *ev) {
    char time_buf[64];
    char rank[LOGGER_MAX_RANK_CHARS + 1] = "\0";

    time_buf[strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    if (L.rank != LOGGER_UNKNOWN_RANK)
        snprintf(rank, LOGGER_MAX_RANK_CHARS + 1, "%d", L.rank);

    fprintf(
            ev->udata, "[%.*s%s] %s %-5s %s:%d: ",
            (LOGGER_MAX_RANK_CHARS < strlen(rank)) ? 0 : (int) (LOGGER_MAX_RANK_CHARS - strlen(rank)),
            "--------------------------------", rank, time_buf, level_strings[ev->level], ev->file, ev->line);
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}

static void lock(void) {
    if (L.lock) { L.lock(true, L.udata); }
}

static void unlock(void) {
    if (L.lock) { L.lock(false, L.udata); }
}

const char *log_level_string(int level) {
    return level_strings[level];
}

int log_level_int(const char *level) {
    char level_copy[8];

    strncpy(level_copy, level, 8);
    for (uint i = 0; i < strlen(level_copy); ++i) {
        if (level_copy[i] >= 'a' && level_copy[i] <= 'z') {
            level_copy[i] = (char) (level_copy[i] - 32);
        }
    }

    if (strncmp("TRACE", level_copy, 8) == 0) return LOG_TRACE;
    if (strncmp("DEBUG", level_copy, 8) == 0) return LOG_DEBUG;
    if (strncmp("INFO", level_copy, 8) == 0) return LOG_INFO;
    if (strncmp("WARN", level_copy, 8) == 0) return LOG_WARN;
    if (strncmp("ERROR", level_copy, 8) == 0) return LOG_ERROR;
    if (strncmp("FATAL", level_copy, 8) == 0) return LOG_FATAL;

    return LOG_INFO;
}

void log_set_lock(log_LockFn fn, void *udata) {
    L.lock = fn;
    L.udata = udata;
}

void log_set_level(int level) {
    L.level = level;
}

void log_set_rank(int rank) {
    L.rank = rank;
}

void log_set_quiet(bool enable) {
    L.quiet = enable;
}

void log_set_colors(bool enable) {
    L.colors = enable;
}

int log_add_callback(log_LogFn fn, void *udata, int level) {
    for (int i = 0; i < LOGGER_MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = (Callback) {fn, udata, level};
            return 0;
        }
    }
    return -1;
}

int log_add_fp(FILE *fp, int level) {
    return log_add_callback(file_callback, fp, level);
}

static void init_event(log_Event *ev, void *udata) {
    if (!ev->time) {
        time_t t = time(NULL);
        ev->time = localtime(&t);
    }
    ev->udata = udata;
}

void log_log(int level, const char *file, int line, const char *fmt, ...) {
    log_Event ev = {
            .fmt   = fmt,
            .file  = file,
            .line  = line,
            .level = level,
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stdout);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < LOGGER_MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}

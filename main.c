#define _BSD_SOURCE
#include <getopt.h>
#include <limits.h>
#include <sqlite3.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syslimits.h>
// #include <linux/limits.h>
#include "allocator.h"

#include "errors.h"
// #define DEBUG 1

#define TS_SIZE 10
#define SBUFF_SIZE 4096

typedef struct {
    uint32_t ts;
    char cmd[SBUFF_SIZE];
    size_t s;
} Entry;

// plusieurs taille
// taille total allouée
// taille des données utiles dans le buffer
typedef struct {
    // curseur, position dans le buffer
    size_t c;
    // taille totale allouée pour le buffer
    size_t _s;
    // taille des données utiles
    size_t s;
    // data
    char *d;
} Buffer;

void Buffer_init(Buffer *buff, char *mem, size_t size)
{
    buff->s = 0;
    buff->c = 0;
    buff->d = mem;
    buff->_s = size;
    memset(buff->d, 0, buff->_s);
}

void Buffer_clear(Buffer *buff)
{
    memset(buff->d, 0, buff->_s);
    buff->s = 0;
    buff->c = 0;
}

void Buffer_cp(Buffer *dst, Buffer *src)
{
    dst->d[dst->c] = src->d[src->c];
    src->c++;
    dst->c++;
    dst->s++;
}

int Buffer_peek(Buffer *b)
{
    if (b->c + 1 >= b->s)
        return EOF;
    else
        return b->d[b->c + 1];
}

void Buffer_dump(Buffer *dst, Buffer *src, size_t size)
{
    memcpy(dst->d, src->d + src->c, size);
    dst->c = size;
    dst->s = size;
    src->c += size;
}

#define PERFCOUNT 1
#define VERB_SIZE 4
typedef enum { VERB_INVALID = -1, VERB_ADD = 0, VERB_LIST = 1 } VERB;
typedef enum {
    FORMAT_INVALID = -1,
    FORMAT_RAW = 0,
    FORMAT_FC = 1,
    FORMAT_ZSH = 2
} FORMAT;
typedef enum { BY_INVALID = -1, BY_DATE = 0, BY_FREQ = 1 } BY;
static char verb_map[][5] = {"add", "list"};
typedef struct {
    int64_t from;
    int64_t to;
    FORMAT format;
    BY by;
    bool order;
    char database[PATH_MAX];
    char hostname[0xff];
} Param;
#define MAX_STREAM_SIZE (size_t)0xffffffff
static const char *SQLITE_MAGIC = "SQLite format 3";
#define MSG_SIZE 4096
static char _msg[MSG_SIZE] = {0};

int sanity_check(const char *buff, size_t size)
{
    for (size_t i = 0; i <= size; i++) {
        if (buff[i] == '\0')
            return 0;
    }

    return -1;
}

void msg_set(const char *format, ...)
{
    va_list arg;
    memset(_msg, 0, MSG_SIZE);
    va_start(arg, format);
    vsnprintf(_msg, MSG_SIZE, format, arg);
    va_end(arg);
}

void msg_print(FILE *stream)
{
    fprintf(stream, "%s\n", _msg);
}

void chrono(float *p, struct timespec *start, struct timespec *end)
{
    *p += (end->tv_sec - start->tv_sec) +
        (end->tv_nsec - start->tv_nsec) / 1000000000.f;
}

void unescape(const char *rbuff, size_t size, char *wbuff)
{
    size_t n = 0;
    size_t bp = 0;

    while (n < size) {
        if (rbuff[n] == '\\') {
            if (n + 1 < size) {
                if (rbuff[n + 1] != '\n' && rbuff[n + 1] != '\\')
                    wbuff[bp++] = rbuff[n];
            }
        } else if (rbuff[n] != '\n')
            wbuff[bp++] = rbuff[n];

        n++;
    }
}

int _insert_hostname(sqlite3 *db, char *hostname, int64_t *host_id)
{
    char *sql = "SELECT id FROM host WHERE val = ?;";
    sqlite3_stmt *stmt = NULL;
    int rc = 0;
    char *msg = NULL;
    int status = 0;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
        goto ERROR;

    sqlite3_bind_text(stmt, 1, hostname, strnlen(hostname, 0xff), NULL);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_ROW) {
        sqlite3_finalize(stmt);
        sql = "INSERT INTO host (val) VALUES (?) RETURNING id;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

        if (rc != SQLITE_OK)
            goto ERROR;

        sqlite3_bind_text(stmt, 1, hostname, strnlen(hostname, 0xff), NULL);
        rc = sqlite3_step(stmt);

        if (rc != SQLITE_ROW)
            goto ERROR;
    }

    *host_id = sqlite3_column_int(stmt, 0);
    goto CLEANUP;
ERROR:
    msg = (char *)sqlite3_errmsg(db);
    msg_set(msg);
    status = -1;
    goto CLEANUP;
CLEANUP:
    sqlite3_finalize(stmt);

    if (msg != NULL)
        free(msg);

    return status;
}

int _insert_cmd(sqlite3 *db, char *cmd, int64_t *cmd_id)
{
    char *msg = NULL;
    int status = 0;
    int rc = 0;
    sqlite3_stmt *stmt = NULL;
    char *sql = "SELECT id FROM cmd WHERE val = ?;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
        goto ERROR;

    sqlite3_bind_text(stmt, 1, cmd, strnlen(cmd, SBUFF_SIZE), NULL);
    int step = sqlite3_step(stmt);

    if (step == SQLITE_ROW)
        *cmd_id = sqlite3_column_int(stmt, 0);
    else {
        sqlite3_finalize(stmt);
        sql = "INSERT INTO cmd (val) VALUES (?) RETURNING id;";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

        if (rc != SQLITE_OK)
            goto ERROR;

        sqlite3_bind_text(stmt, 1, cmd, strnlen(cmd, SBUFF_SIZE), NULL);
        rc = sqlite3_step(stmt);

        if (rc != SQLITE_ROW)
            goto ERROR;
    }

    *cmd_id = sqlite3_column_int(stmt, 0);
    goto CLEANUP;
ERROR:
    msg = (char *)sqlite3_errmsg(db);
    msg_set(msg);
    status = -1;
    goto CLEANUP;
CLEANUP:
    // if (msg != NULL)
    //     free(msg);
    sqlite3_finalize(stmt);
    return status;
}

int _insert_hist(sqlite3 *db, int64_t host_id, int64_t cmd_id, time_t ts)
{
    int status = 0;
    int rc = 0;
    sqlite3_stmt *stmt = NULL;
    char *msg = NULL;
    char *sql = "INSERT INTO hist (host_id, cmd_id, ts) VALUES (?, ?, ?) ON "
        "CONFLICT DO NOTHING;";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
        goto ERROR;

    sqlite3_bind_int64(stmt, 1, host_id);
    sqlite3_bind_int64(stmt, 2, cmd_id);
    sqlite3_bind_int64(stmt, 3, ts);
    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE)
        goto ERROR;

    goto CLEANUP;
ERROR:
    msg = (char *)sqlite3_errmsg(db);
    msg_set(msg);
    status = -1;
    goto CLEANUP;
CLEANUP:

    if (msg != NULL)
        free(msg);

    sqlite3_finalize(stmt);
    return status;
}

int db_hist(sqlite3 *db, Entry *entry, size_t esize, Param *param)
{
    size_t ep = 0;
    int status = 0;
    int64_t host_id = 0;

    if (_insert_hostname(db, param->hostname, &host_id) < 0)
        goto ERROR;

    while (ep < esize) {
        uint32_t ts = *(uint32_t *)&entry[ep].ts;
        int64_t cmd_id = 0;
        char *cmd = entry[ep].cmd;

        if (_insert_cmd(db, cmd, &cmd_id) < 0)
            goto ERROR;

        if (_insert_hist(db, host_id, cmd_id, ts) < 0)
            goto ERROR;

        ep++;
    }

    goto CLEANUP;
ERROR:
    status = -1;
    goto CLEANUP;
CLEANUP:
    return status;
}

int db_add(sqlite3 *db, char *buff, const size_t buffsize, Param *param)
{
    time_t ts = time(NULL);
    int64_t host_id = 0;
    int64_t cmd_id = 0;
    char cmd[SBUFF_SIZE] = {0};
    int status = 0;

    if (buffsize >= SBUFF_SIZE) {
        msg_set(ERR_STREAM_OVERFLOW, SBUFF_SIZE);
        goto ERROR;
    }

    unescape(buff, buffsize, cmd);

    if (_insert_hostname(db, param->hostname, &host_id) < 0)
        goto ERROR;

    if (_insert_cmd(db, cmd, &cmd_id) < 0)
        goto ERROR;

    if (_insert_hist(db, host_id, cmd_id, ts) < 0)
        goto ERROR;

    goto CLEANUP;
ERROR:
    status = -1;
    goto CLEANUP;
CLEANUP:
    return status;
}

int db_list(sqlite3 *db, Param *param)
{
    int status = 0;
    char *msg = NULL;
    sqlite3_stmt *stmt;
    int rc = 0;
    int n = 1;
    char sql[4096] = "SELECT DISTINCT(c.val) "
        "FROM hist AS h "
        "JOIN host AS ho ON (h.host_id = ho.id) "
        "JOIN cmd AS c ON (h.cmd_id = c.id) "
        "WHERE h.ts >= ? "
        "AND h.ts <= ? ";

    if (strcmp(param->hostname, "all") != 0) {
        const char *str = "AND ho.val LIKE ? ";
        strncat(sql, str, strlen(str));
    }

    if (param->by == BY_DATE) {
        const char *str = "ORDER BY h.ts ";
        strncat(sql, str, strlen(str));
    } else if (param->by == BY_FREQ) {
        const char *str = "ORDER BY h.ts ";
        strncat(sql, str, strlen(str));
    }

    if (param->order == true)
        strncat(sql, "ASC", 3);
    else
        strncat(sql, "DESC", 4);

    strncat(sql, ";", 1);
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);

    if (rc != SQLITE_OK)
        goto ERROR;

    sqlite3_bind_int64(stmt, 1, (int64_t)param->from);
    sqlite3_bind_int64(stmt, 2, (int64_t)param->to);
    sqlite3_bind_text(stmt, 3, param->hostname, strlen(param->hostname), NULL);

    while (sqlite3_step(stmt) == SQLITE_ROW)
        fprintf(stdout, "% 5d %s\n", n++, sqlite3_column_text(stmt, 0));

    goto CLEANUP;
ERROR:
    msg = (char *)sqlite3_errmsg(db);
    msg_set(msg);
    status = -1;
    goto CLEANUP;
CLEANUP:

    if (msg != NULL)
        free(msg);

    sqlite3_finalize(stmt);
    return status;
}

int db_init(sqlite3 **db, const char *path)
{
    int status = 0;
    char *msg = NULL;
    // db loading, create new file if none
    int rc = sqlite3_open(path, db);

    if (rc != SQLITE_OK) {
        msg = (char *)sqlite3_errmsg(*db);
        msg_set(msg);
        goto ERROR;
    }

    // db initialization if schema does not exists
    char *sql = "CREATE TABLE IF NOT EXISTS host (id INTEGER NOT NULL PRIMARY "
        "KEY AUTOINCREMENT, val NVARCHAR(250) UNIQUE NOT NULL);"
        "CREATE TABLE IF NOT EXISTS cmd (id INTEGER NOT NULL PRIMARY KEY "
        "AUTOINCREMENT, val NVARCHAR(250) UNIQUE NOT NULL);"
        "CREATE TABLE IF NOT EXISTS hist (id INTEGER NOT NULL PRIMARY "
        "KEY AUTOINCREMENT, host_id INTEGER NOT NULL, cmd_id INTEGER NOT "
        "NULL, ts INTEGER NOT NULL, FOREIGN KEY(host_id) REFERENCES "
        "host(id), FOREIGN KEY(cmd_id) REFERENCES cmd(id));";
    // "CREATE TABLE IF NOT EXISTS hist (host_id INTEGER NOT NULL, cmd_id INTEGER
    // NOT NULL, ts INTEGER NOT NULL, FOREIGN KEY(host_id) REFERENCES host(id),
    // FOREIGN KEY(cmd_id) REFERENCES cmd(id), PRIMARY KEY (ts, host_id,
    // cmd_id));";
    rc = sqlite3_exec(*db, sql, 0, 0, &msg);

    if (rc != SQLITE_OK) {
        msg_set(msg);
        goto ERROR;
    }

    goto CLEANUP;
ERROR:
    status = -1;
    goto CLEANUP;
CLEANUP:

    if (msg != NULL)
        free(msg);

    return status;
}

int parser_arg(int c, char *optarg, VERB verb, Param *param)
{
    switch (c) {
    case 'h':
        goto ARG_HOSTNAME;

    case 'd':
        goto ARG_DATABASE;

    default:
        if (verb == VERB_ADD)
            goto STATE_ADD;
        else if (verb == VERB_LIST)
            goto STATE_LIST;
    }

STATE_ADD:

    if (c == 'r')
        goto ARG_FORMAT_ADD;
    else
        goto ARG_INVALID;

STATE_LIST:

    if (c == 'r')
        goto ARG_FORMAT_LIST;
    else if (c == 'f')
        goto ARG_FROM;
    else if (c == 't')
        goto ARG_TO;
    else if (c == 'b')
        goto ARG_BY;
    else
        goto ARG_INVALID;

ARG_INVALID:
    msg_set(ERR_OPT_INVALID, c, verb_map[verb]);
    return -1;
ARG_DATABASE : {
        memset(param->database, 0, PATH_MAX);
        FILE *fp = fopen(optarg, "r");

        if (fp == NULL) {
            msg_set(ERR_DB_PATH, optarg);
            return -1;
        }

        char buff[16] = {0};
        fread(buff, 1, 16, fp);

        if (strcmp(buff, SQLITE_MAGIC) != 0) {
            msg_set(ERR_DB_INVALID, buff, optarg);
            return -1;
        }

        strcpy(param->database, optarg);
        return 0;
    }
ARG_HOSTNAME:

    if (sanity_check(optarg, 0xff) < 0) {
        msg_set(ERR_HOST_INVALID);
        return -1;
    } else {
        memset(param->hostname, 0, 0xff);
        strcpy(param->hostname, optarg);
        return 0;
    }

ARG_FORMAT_ADD:

    if (sanity_check(optarg, 4) < 0) {
        msg_set(ERR_FORMAT_INVALID, optarg);
        return -1;
    } else if (strcmp(optarg, "raw") == 0) {
        param->format = FORMAT_RAW;
        return 0;
    } else if (strcmp(optarg, "zsh") == 0) {
        param->format = FORMAT_ZSH;
        return 0;
    } else {
        msg_set(ERR_FORMAT_UNKNOWN, optarg);
        return -1;
    }

ARG_FORMAT_LIST:

    if (sanity_check(optarg, 4) < 0) {
        msg_set(ERR_FORMAT_INVALID, optarg);
        return -1;
    } else if (strcmp(optarg, "fc") == 0) {
        param->format = FORMAT_FC;
        return 0;
    } else {
        msg_set(ERR_FORMAT_UNKNOWN, optarg);
        return -1;
    }

ARG_FROM:

    if (sanity_check(optarg, 10) < 0) {
        msg_set(ERR_TS_INVALID, optarg);
        return -1;
    } else {
        char *p = NULL;
        long val = strtol(optarg, &p, 10);

        if (p == optarg) {
            msg_set(ERR_TS_INVALID, optarg);
            return -1;
        }

        param->from = val;
        return 0;
    }

ARG_TO:

    if (sanity_check(optarg, 10) < 0) {
        msg_set(ERR_TS_INVALID, optarg);
        return -1;
    } else {
        char *p = NULL;
        long val = strtol(optarg, &p, 10);

        if (p == optarg) {
            msg_set(ERR_TS_INVALID, optarg);
            return -1;
        }

        param->to = val;
        return 0;
    }

ARG_BY:

    if (sanity_check(optarg, 4) < 0) {
        msg_set(ERR_BY_INVALID);
        return -1;
    } else if (strcmp(optarg, "date") == 0) {
        param->by = BY_DATE;
        return 0;
    } else if (strcmp(optarg, "freq") == 0) {
        param->by = BY_FREQ;
        return 0;
    } else {
        msg_set(ERR_BY_UNKOWN, optarg);
        return -1;
    }
}

int parser_histfile_zsh(Buffer *rbuff, Entry *entry, size_t *esize)
{
    char arr[SBUFF_SIZE] = {0};
    char _sbuff[SBUFF_SIZE] = {0};
    Buffer sbuff = {.d = _sbuff, .c = 0, .s = 0, ._s = SBUFF_SIZE};
    uint32_t ts = 0;
    size_t ep = 0;
    Handle handles[1000] = {0};
    goto STATE_NEWLINE;
STATE_SAVE:

    // current position strictly inferior to total allocated size
    if (ep < *esize) {
        Handle h = allocator_alloc(sizeof(Entry));
        // memcpy(entry[ep].cmd, sbuff.d, sbuff.s);
        Entry e = {.s = sbuff.s, .ts = ts, .cmd = {0, 0, 0, 0, 0, 0}};
        mem_write(h, &e, handle_size(h));
        handles[ep++] = h;

        if (rbuff->c >= rbuff->s) {
            *esize = ep;
            goto STATE_DONE;
        }

        goto STATE_TS;
    } else
        goto STATE_MEM;

STATE_NEWLINE:

    if (rbuff->d[rbuff->c] != ':')
        goto STATE_CMD;

    if (sbuff.s)
        goto STATE_SAVE;

    Buffer_clear(&sbuff);
    goto STATE_TS;
STATE_TS:
    memset(arr, 0, SBUFF_SIZE);
    rbuff->c += 2;
    Buffer_dump(&sbuff, rbuff, TS_SIZE);
    rbuff->c += 3;
    ts = (int)strtol(sbuff.d, NULL, 10);
    Buffer_clear(&sbuff);
    goto STATE_CMD;
STATE_CMD:

    if (rbuff->c >= rbuff->s) {
        ;
        goto STATE_SAVE;
    } else if (rbuff->d[rbuff->c] == '\n') {
        rbuff->c++;
        goto STATE_NEWLINE;
    } else if (rbuff->d[rbuff->c] == '\\') {
        ;
        goto STATE_ESCAPE;
    } else {
        Buffer_cp(&sbuff, rbuff);
        goto STATE_CMD;
    }

STATE_ESCAPE:
    ;
    // unhandled case -1
    int val = Buffer_peek(rbuff);

    switch (val) {
    // backslash + LF + LF
    // backslash + LF
    case '\n':
        rbuff->c += 2;
        goto STATE_NEWLINE;
        break;

    // backslash + backslash
    case '\\':
        rbuff->c++;
        goto STATE_ESCAPE;
        break;

    case EOF:
        goto STATE_NEWLINE;

    // backslash + character
    default:
        Buffer_cp(&sbuff, rbuff);
        goto STATE_CMD;
    }

STATE_DONE:
    return 1;
STATE_MEM:
    ;
    fprintf(stderr, "CRITICAL_SECTION\n");
    return -1;
    *esize *= 2;
    Entry *p = realloc(entry, sizeof(Entry) * *esize);

    if (p == NULL) {
        fprintf(stderr, "Failed to re-allocate working mem\n");
        return -1;
    } else
        goto STATE_SAVE;
}

int order_flag = 0;

int main(int argc, char **argv)
{
    char *buff = NULL;
    size_t buffsize = 0;
    sqlite3 *db = NULL;
    int status = EXIT_SUCCESS;
    Param param = {.from = 0x0,
              .to = 0xffffffff,
              .by = BY_DATE,
              .database = {0},
              .hostname = {0},
              .order = false
          };
    // default hostname
    gethostname(&param.hostname[0], 0xff);
    char path[SBUFF_SIZE] = {0};
    {
        const char *workspace = getenv("WORKSPACE");

        if (!workspace) {
            msg_set("WORKSPACE is not defined");
            goto ERROR;
        }

        const char *data = "/.cache/db.sqlite";
        strcat(path, workspace);
        strcat(path, data);
    }
    // default database
    strcpy(&param.database[0], path);
    VERB verb;

    if (argc < 2) {
        msg_set(ERR_USAGE);
        goto ERROR;
    }

    if (sanity_check(argv[1], VERB_SIZE) < 0) {
        msg_set(ERR_VERB_INVALID, argv[1]);
        goto ERROR;
    }

    if (strcmp(argv[1], "add") == 0) {
        verb = VERB_ADD;
        param.format = FORMAT_RAW;
    } else if (strcmp(argv[1], "list") == 0) {
        verb = VERB_LIST;
        param.format = FORMAT_FC;
    } else {
        msg_set(ERR_VERB_UNKNOWN, argv[1]);
        goto ERROR;
    }

    static struct option long_options[] = {
        {"order", no_argument, &order_flag, 1},
        {"format", required_argument, 0, 'r'},
        {"from", required_argument, 0, 'f'},
        {"to", required_argument, 0, 't'},
        {"by", required_argument, 0, 'b'},
        {"hostname", required_argument, 0, 'h'},
        {"database", required_argument, 0, 'd'},
        {NULL, 0, NULL, 0}
    };

    while (1) {
        int option_index = 0;
        int c =
            getopt_long(argc, argv, "r:f:t:b:h:d:", long_options, &option_index);

        if (c == -1)
            break;
        else if (c == 0 && long_options[option_index].flag != 0) {
            param.order = 1;
            // if (optarg)
            //     goto ERROR;
        } else if (parser_arg(c, optarg, verb, &param) < 0)
            goto ERROR;
    }

    if (verb == VERB_ADD) {
        // consumes STDIN in buff, allocates accordingly
        buff = malloc(sizeof(char) * SBUFF_SIZE);

        if (buff == NULL) {
            msg_set(ERR_MEM);
            goto ERROR;
        }

        memset(buff, 0, SBUFF_SIZE);
        int c;
        size_t n = 1;
        char *_buff = NULL;

        while ((c = getchar()) != EOF) {
            buff[buffsize++] = (char)c;

            if (buffsize >= MAX_STREAM_SIZE) {
                msg_set(ERR_STREAM_OVERFLOW, MAX_STREAM_SIZE);
                goto ERROR;
            } else if (buffsize >= SBUFF_SIZE * n) {
                _buff = realloc(buff, SBUFF_SIZE * ++n);

                if (_buff == NULL) {
                    msg_set(ERR_MEM);
                    goto ERROR;
                } else
                    buff = _buff;
            }
        }
    }

#ifdef DEBUG
    printf("from=%lld to=%lld format=%d order=%d database=%s hostname=%s\n",
        param.from, param.to, param.format, param.order, param.database,
        param.hostname);
    printf("buff=");

    for (size_t i = 0; i < buffsize; i++)
        printf("%c", buff[i]);

#endif
    //
    // schlag patch pour adapter à l'interface du parser
    //
    Buffer rbuff = {.c = 0, ._s = buffsize, .s = buffsize, .d = buff};
    int lines = 0;
    size_t esize = 0;
    Entry *entry = NULL;
    {
        while (rbuff.c < rbuff.s) {
            if (rbuff.d[rbuff.c] == '\n')
                lines++;

            rbuff.c++;
        }

        rbuff.c = 0;
        entry = malloc(sizeof(Entry) * lines);

        if (entry == NULL) {
            msg_set("Could not allocate memory for buffer\n");
            goto ERROR;
        }

        memset(entry, 0, sizeof(Entry) * lines);
        esize = lines;
    }

    if (db_init(&db, param.database) < 0) {
        ;
        goto ERROR;
    } else if (verb == VERB_ADD) {
        if (param.format == FORMAT_RAW) {
            ;
            db_add(db, buff, buffsize, &param);
        } else if (param.format == FORMAT_ZSH) {
            parser_histfile_zsh(&rbuff, entry, &esize);
            db_hist(db, entry, esize, &param);
        }
    } else if (verb == VERB_LIST) {
        ;
        db_list(db, &param);
    } else {
        msg_set("Verb %s not implemented", verb_map[verb]);
        goto ERROR;
    }

    goto CLEANUP;
ERROR:
    status = EXIT_FAILURE;
    msg_print(stderr);
    goto CLEANUP;
CLEANUP:

    if (buff != NULL)
        free(buff);

    if (db != NULL)
        sqlite3_close(db);

    exit(status);
}

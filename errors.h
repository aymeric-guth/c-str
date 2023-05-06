#ifndef ERRORS_H
#define ERRORS_H

static const char *ERR_TS_INVALID = "Invalid value for `timestamp` %d";
static const char *ERR_BY_INVALID = "Invalid value for `by`";
static const char *ERR_BY_UNKOWN = "Unknown value for `by` %s";
static const char *ERR_OPT_INVALID = "Invalid option `%c` for `%s`";
static const char *ERR_DB_PATH = "SQLite file not found %s";
static const char *ERR_DB_INVALID = "buff=%s\n %s is not a valid SQLite database";
static const char *ERR_HOST_INVALID = "Invalid value for hostname";
static const char *ERR_FORMAT_UNKNOWN = "Unknown format: %s";
static const char *ERR_FORMAT_INVALID = "Invalid value for format";
static const char *ERR_USAGE = "usage: cli verb [--opt arg]";
static const char *ERR_VERB_INVALID = "Invalid verb %s";
static const char *ERR_VERB_UNKNOWN = "Unknown verb %s";
static const char *ERR_MEM = "Could not allocate memory";
static const char *ERR_STREAM_OVERFLOW = "Stream is too large, limit is %lu";
#endif

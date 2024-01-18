#include "sqlite3-wrapper.h"

SQLite::SQLite(const char * filename):_filename(filename) {
    reset();
    printf("Open %s ", filename);
    int rc = sqlite3_open(filename, &_db);
    if (rc) {
        error_message("sqlite3_open");
        reset();
        exit(0);
    }
}

SQLite::~SQLite() {
    reset();
}

void SQLite::reset() {
    _reset_stmt();
    if (_db) {
        sqlite3_close(_db);
        _db = nullptr;
    }
}

void SQLite::_reset_stmt() {
    _column_count = 0;
    if (_stmt) {
        sqlite3_finalize(_stmt);
        _stmt = nullptr;
    }
    if (_row) {
        delete[] _row;
        _row = nullptr;
    }
    if (_column_names) {
        delete[] _column_names;
        _column_names = nullptr;
    }
}

// SQL methods
// all va_args are const char *

int SQLite::_prepare(const char * sql, va_list ap) {
    _reset_stmt();
    int rc = sqlite3_prepare_v2(_db, sql, -1, &_stmt, nullptr);
    if (rc) {
        error_message("sqlite3_prepare_v2");
        _reset_stmt();
        return 0;
    }
    _column_count = sqlite3_column_count(_stmt);
    int parameter_count = sqlite3_bind_parameter_count(_stmt);
    if (parameter_count) {
        for(int i = 1; i <= parameter_count; ++i) {     // params start at 1
            const char * parameter = va_arg(ap, const char *);
            sqlite3_bind_text(_stmt, i, parameter, -1, SQLITE_STATIC);
        }
    }
    return parameter_count;
}

int SQLite::select(const char * sql, ...) {
    va_list ap;
    va_start(ap, sql);
    _prepare(sql, ap);
    va_end(ap);
    
    return column_count();
}

int SQLite::execute(const char * sql, ...) {
    va_list ap;
    va_start(ap, sql);
    _prepare(sql, ap);
    va_end(ap);
    
    sqlite3_step(_stmt);
    _reset_stmt();
    return sqlite3_changes(_db);
}

const char * SQLite::value(const char * sql, ...) {
    va_list ap;
    va_start(ap, sql);
    _prepare(sql, ap);
    va_end(ap);
    
    const char ** row = fetch_row();
    if (row) {
        return row[0];
    } else {
        return nullptr;
    }
}

const char ** SQLite::fetch_row() {
    // do we have a statement?
    if (!_stmt) {
        _reset_stmt();
        return nullptr;
    }
    // get the next row, if avail
    if (sqlite3_step(_stmt) != SQLITE_ROW) {
        _reset_stmt();
        return nullptr;
    }
    // make sure we have allocated space
    if (_column_count && !_row) {
        _row = new const char * [_column_count];
    }
    for (int index = 0; index < _column_count; ++index) {
        _row[index] = (const char *) sqlite3_column_text(_stmt, index);
    }
    return _row;
}

const char ** SQLite::column_names() {
    if (!_stmt) {
        _reset_stmt();
        return nullptr;
    }
    if (_column_count && !_column_names) {
        _column_names = new const char * [_column_count];
    }
    for (int index = 0; index < _column_count; ++index) {
        _column_names[index] = (const char *) sqlite3_column_name(_stmt, index);
    }
    return _column_names;
}

int SQLite::column_count() {
    return _column_count;
}

const char * SQLite::filename() {
    return _filename;
}

void SQLite::error_message(const char * str) {
    if (str) {
        printf("%s: ", str);
    }
    if (_db) {
        printf("%s\n", sqlite3_errmsg(_db));
    } else {
        printf("Unknown error\n");
    }
}

#ifndef SQLITE3_WRAPPER_H
#define SQLITE3_WRAPPER_H

#include <sqlite3.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <memory>

class SQLite {
    const char * _filename = nullptr;
    sqlite3 * _db = nullptr;
    sqlite3_stmt * _stmt = nullptr;
    int _column_count = 0;
    const char ** _column_names = nullptr;
    const char ** _row =  nullptr;

public:
    SQLite(const char * filename);
    ~SQLite();
    void reset();

    // sql methods
    int select(const char * sql, ...);
    const char** fetch_row();
    const char** column_names();
    int column_count();

    int execute(const char * sql, ...);
    const char * value(const char * sql, ...);
    
    const char * filename();
    void error_message(const char * str = nullptr);

private:
    int _prepare(const char * sql, va_list ap);
    void _reset_stmt();
};

#endif // SQLite_H

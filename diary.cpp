#include <cstdio>
#include "sqlite3-wrapper.h"
#include "diary.h"

constexpr const char* sql_create_table =
"CREATE TABLE IF NOT EXISTS books ("
"id INTEGER PRIMARY KEY AUTOINCREMENT,"
"title VARCHAR(127) CHECK(title != ''),"
"first_name VARCHAR(127) CHECK(first_name != ''),"
"last_name VARCHAR(127) CHECK(last_name != '')"
")"
;
constexpr const char* sql_insert = "INSERT INTO books (title, first_name, last_name) VALUES (?, ?, ?)";
constexpr const char* sql_drop_table = "DROP TABLE IF EXISTS books";
constexpr const char* sql_delete_by_id = "DELETE FROM books WHERE id = ?";
constexpr const char* sql_list = "SELECT * FROM books";
constexpr const char* sql_list_by_author = "SELECT * FROM books ORDER BY last_name";
constexpr const char* sql_list_by_title = "SELECT * FROM books ORDER BY title";
constexpr const char* sql_find_by_author = "SELECT * FROM books WHERE làst_name LIKE ?";
constexpr const char* sql_find_by_title = "SELECT * FROM books WHERE title LIKE ?";
constexpr const char* sql_check_book_table = "SELECT name FROM sqlite_master WHERE type = 'table' AND name = 'books'";

constexpr const char* menu[] = {
    "0) List books",
    "1) List books by author",
    "2) List books by title",
    "3) Add books",
    "4) Find books by author",
    "5) Find books by title",
    "6) Delete book by id",
    "X) Drop table and exit",
    "Q) Quit"
};

constexpr const char* prompt = "Select an action or Q to quit";

const char* promptline(const char* prompt) {
    static char buf[MAX_SMALL_STRING_LENGTH];
    printf("%s > ", prompt);
    memset(buf, 0, MAX_SMALL_STRING_LENGTH);
    if (fgets(buf, MAX_SMALL_STRING_LENGTH, stdin) != NULL) {
        buf[strcspn(buf, "\n")] = '\0'; // trim CR/LF
    }
    return buf;
}

void do_menu(SQLite& db) {
    char response = 'x';
    const char* buf = nullptr;
    while (true) {
        puts("");
        printf("Current database %s:\n", db.filename());
        // print the menu
        for (const char* s : menu) {
            puts(s);
        }
        // get the response
        buf = promptline(prompt);
        size_t bufsize = strnlen(buf, MAX_SMALL_STRING_LENGTH);
        // one character only, please
        if (bufsize != 1) {
            puts("Input too long or empty");
            continue;
        }
        response = buf[0];
        // process the response
        if (response >= 'a' && response <= 'z') {   // make it uppercase
            response -= 0x20;
        }
        // check if we know this one
        if (!strchr("0123456XQ", response)) {
            puts("Invalid response");
            continue;
        }
        // we can handle this one right here
        if (response == 'Q') {
            break;
        }
        // do it
        do_jump(db, response);
    }
}

void do_jump(SQLite& db, const char response) {
    switch (response) {
    case '0':
        do_list(db);
        break;
    case '1':
        do_list_by_author(db);
        break;
    case '2':
        do_list_by_title(db);
        break;
    case '3':
        do_add(db);
        break;
    case '4':
        do_find_by_author(db);
        break;
    case '5':
        do_find_by_title(db);
        break;
    case '6':
        do_delete(db);
        break;
    case 'X':
        do_drop_exit(db);
        break;
    default:
        break;
    }
}

void display_rows(SQLite& db) {
    int count = 0;
    for (const char** row = db.fetch_row(); row; row = db.fetch_row()) {
        for (int i = 0; i < db.column_count(); ++i) {
            printf("%s%s", row[i], (i < db.column_count() - 1) ? "; " : "\n");
        }
        count++;
    }
    if (count == 0) {
        puts("Table is empty");
    }
}

void do_list(SQLite& db) {
    puts("List books:");
    db.select(sql_list);
    display_rows(db);
}

void do_list_by_author(SQLite& db) {
    puts("List books ordered by author:");
    db.select(sql_list_by_author);
    display_rows(db);
}

void do_list_by_title(SQLite& db) {
    puts("List books ordered by title:");
    db.select(sql_list_by_title);
    display_rows(db);
}

void do_add(SQLite& db) {
    const char* buf = nullptr;

    char title[MAX_SMALL_STRING_LENGTH];
    char first_name[MAX_SMALL_STRING_LENGTH];
    char last_name[MAX_SMALL_STRING_LENGTH];

    puts("Add book:");
    buf = promptline("Title");
    strncpy_s(title, buf, strlen(buf) + 1);
    buf = promptline("Author first name");
    strncpy_s(first_name, buf, strlen(buf) + 1);
    buf = promptline("Author last name");
    strncpy_s(last_name, buf, strlen(buf) + 1);
    int rc = db.execute(sql_insert, title, first_name, last_name);
    if (!rc) {
        db.error_message("Could not add row");
    }
}

void do_find_by_author(SQLite& db) {
    const char* buf = promptline("Find by author last name");
    db.select(sql_find_by_author, buf);
    display_rows(db);
}

void do_find_by_title(SQLite& db) {
    const char* buf = promptline("Find by title");
    db.select(sql_find_by_title, buf);
    display_rows(db);
}

void do_delete(SQLite& db) {
    const char* id = nullptr;
    puts("Add book:");
    id = promptline("Delete book with ID");
    int rc = db.execute(sql_delete_by_id, id);
    if (!rc) {
        db.error_message("Could not delete row");
    }
}

void do_drop_exit(SQLite& db) {
    db.execute(sql_drop_table);
    exit(0);
}

int main() {
    const char* filename = nullptr;
    puts("Create (or open if exists) database:");
    filename = promptline("Database filename (default diary.db)");
    size_t filename_size = strnlen(filename, MAX_SMALL_STRING_LENGTH);
    if (filename_size == 0) {
        filename = "diary.db";
    }
    
    SQLite db(filename);
    const char* rc = db.value(sql_check_book_table);
    if (!rc) {
        db.execute(sql_create_table);
    }
    do_menu(db);
    
    return 0;
}

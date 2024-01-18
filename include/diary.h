#ifndef _DIARY_H
#define _DIARY_H

#define MAX_SMALL_STRING_LENGTH 255

void display_rows(SQLite & db);

const char * promptline(const char * promt);
void do_menu(SQLite & db);
void do_jump(SQLite & db, const char response);

void do_add(SQLite & db);
void do_delete(SQLite & db);

void do_list(SQLite& db);
void do_list_by_author(SQLite & db);
void do_list_by_title(SQLite & db);

void do_find_by_author(SQLite & db);
void do_find_by_title(SQLite & db);

void do_drop_exit(SQLite & db);

#endif

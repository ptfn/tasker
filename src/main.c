#include <sqlite3.h>
#include <stdbool.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VERSION "0.1.1"

typedef struct sql
{
    sqlite3_stmt *stmt;
    sqlite3 *db;
    char *err;
    int rc;
} sql;

typedef struct size
{
    uint16_t x, y;
} size;

sql tasker;

void display_menu(WINDOW *win_menu, uint16_t xMaxM,
                  uint16_t yMax, int8_t i,
                  char *item, char list[3][5])
{
    int8_t j = 0;
    box(win_menu, 0, 0);
    
    mvwprintw(win_menu, 0, xMaxM/2-2, "%s", "Menu");
    mvwprintw(stdscr, yMax-1, 0, "Tasker %s", VERSION);
    
    for (; j < 3; j++) {
        if (i == j)
            wattron(win_menu, A_STANDOUT);
        else
            wattroff(win_menu, A_STANDOUT);
        sprintf(item, "%-4s", list[j]);
        mvwprintw(win_menu, j + 1, 2, "* %s", item);
    }
    
    wrefresh(stdscr);
    wrefresh(win_menu);
}

/* Window Menu */
uint8_t menu(void)
{
    size max, max_menu, temp;
    WINDOW *win_menu;
    int8_t i = 0;
    uint16_t ch;

    char list[3][5] = {"New", "Load", "Exit"};
    char item[5];

    getmaxyx(stdscr, max.y, max.x);
    win_menu = newwin(5, max.x/4, max.y/2-(5/2), max.x/2-(max.x/4)/2);
    getmaxyx(win_menu, max_menu.y, max_menu.x);

    display_menu(win_menu, max_menu.x, max.y, i, item, list);

    noecho();
    keypad(win_menu, TRUE);
    curs_set(0);
    
    i = 0;

    while ((ch = wgetch(win_menu)) != 10) {
        getmaxyx(stdscr, temp.y, temp.x);

        if (temp.y != max.y || temp.x != max.x) {
            max.y = temp.y;
            max.x = temp.x;

            wresize(win_menu, 5, max.x/4);
            mvwin(win_menu, max.y/2-(5/2), max.x/2-(max.x/4)/2);
            getmaxyx(win_menu, max_menu.y, max_menu.x);

            wclear(win_menu);
            wclear(stdscr);
            display_menu(win_menu, max_menu.x, max.y, i, item, list);
        }

        sprintf(item, "%-4s", list[i]);
        mvwprintw(win_menu, i + 1, 2, "* %s", item);

        switch (ch) {
            case KEY_UP:
                i--;
                i = (i < 0) ? 2 : i;
                break;
            case KEY_DOWN:
                i++;
                i = (i > 2) ? 0 : i;
                break;
            case 'q': case 'Q':
                delwin(win_menu);
                endwin();
                exit(0);
        }

        wattron(win_menu, A_STANDOUT);
        sprintf(item, "%-4s", list[i]);
        mvwprintw(win_menu, i + 1, 2, "- %s", item);
        wattroff(win_menu, A_STANDOUT);
    }

    delwin(win_menu);
    endwin();

    return i; 
}

/* Function Open or Create DB */
void load(uint8_t menu_choice, char *name_db)
{
    tasker.err = 0;
    tasker.rc = sqlite3_open(name_db, &tasker.db);

    if (tasker.rc != SQLITE_OK) {
        sqlite3_close(tasker.db);
    }

    if (!(menu_choice)) {
        char *sql = "DROP TABLE IF EXISTS task;"
                    "CREATE TABLE task(id INTEGER PRIMARY KEY AUTOINCREMENT, task TEXT, time_date REAL);";
        tasker.rc = sqlite3_exec(tasker.db, sql, 0, 0, &tasker.err);

        if (tasker.rc != SQLITE_OK) {
            sqlite3_free(tasker.err);
            sqlite3_close(tasker.db);
        }
    }
}

void display_open(WINDOW *win_open, size max_open, const char *title)
{
    wclear(win_open);
    wclear(stdscr);

    box(win_open, 0, 0);

    mvwprintw(win_open, 0, max_open.x/2-2, "%s", title);
    mvwprintw(win_open, max_open.y/2, 2, "%s", "Enter:");

    wrefresh(stdscr);
    wrefresh(win_open);
}

/* Window Input Name DB */
char *open(const char *title)
{
    char *str = calloc(100, sizeof(char));
    size max, temp, max_open;
    WINDOW *win_open;

    getmaxyx(stdscr, max.y, max.x); 
    win_open = newwin(5, max.x/2, max.y/2-(5/2), max.x/2-(max.x/2)/2); 
    getmaxyx(win_open, max_open.y, max_open.x);
    display_open(win_open, max_open, title);

    echo();        
    keypad(win_open, TRUE); 
    curs_set(1);

    for (;;) {
        getmaxyx(stdscr, temp.y, temp.x);

        if (max.x != temp.x || max.y != temp.y) {
            max.y = temp.y;
            max.x = temp.x;

            wresize(win_open, 5, max.x/2);
            mvwin(win_open, max.y/2-(5/2), max.x/2-(max.x/2)/2);
            getmaxyx(win_open, max_open.y, max_open.x);
            display_open(win_open, max_open, title);
        }

        mvwgetstr(win_open, 2, 9, str);

        if (strlen(str) == 0)
            break;
    }
    delwin(win_open);
    endwin();

    return str;
}

/* Choice Menu */
void choice(uint8_t menu_choice)
{
    if (menu_choice == 0 || menu_choice == 1) {
        char *path = calloc(100, sizeof(char));
        if (menu_choice)
            path = open("Load");
        else
            path = open("New");

        load(menu_choice, path);
    } else {
        exit(0);
    }
}

/* Print Table On Window */
void print_table(WINDOW *win)
{
    char *sql = "SELECT * FROM task";
    uint16_t i = 1;
    size max;

    tasker.rc = sqlite3_prepare_v2(tasker.db, sql, -1, &tasker.stmt, 0);

    if (tasker.rc != SQLITE_OK) {
        sqlite3_close(tasker.db);
    }

    getmaxyx(win, max.y, max.x);
    mvwprintw(win, 2, 2, "Id  Task");
    mvwprintw(win, 2, max.x-12, "Date");

    while (sqlite3_step(tasker.stmt) == SQLITE_ROW) {
        const unsigned char *_tmp = sqlite3_column_text(tasker.stmt, 1);
        const char *_tmp1 = (const char*)sqlite3_column_text(tasker.stmt, 2);

        mvwprintw(win, 3+(i-1), 2, "%d  %s", i++, _tmp);
        mvwprintw(win, 3+(i-1), max.x-12, "%s", _tmp1);
    }

    sqlite3_finalize(tasker.stmt);
}

/* Command Window Under SQL */
void command(WINDOW *win, const char *command)
{
    char *task = calloc(100, sizeof(char));
    char *sql = calloc(200, sizeof(char));
                
    echo();
    curs_set(1);
                
    wclear(win);
    mvwprintw(win, 0, 1, "Enter (%s): ", command);
    wrefresh(win);
    mvwgetstr(win, 0, 14, task);

    noecho();
    curs_set(0);

    if (!(strcmp(command, "add")))
        sprintf(sql, "INSERT INTO task (task, time_date) VALUES ('%s', date('now'));", task);
    else if (!(strcmp(command, "del")))
        sprintf(sql, "DELETE FROM task WHERE task='%s';", task);

    tasker.rc = sqlite3_exec(tasker.db, sql, 0, 0, &tasker.err);
                
    if (tasker.rc != SQLITE_OK) {
        sqlite3_free(tasker.err);
        sqlite3_close(tasker.db);
    }

    free(task);
    free(sql);
}

/* Main Window */
void task(void)
{
    size max, max_task, max_input __attribute__((unused));
    WINDOW *task, *input;
    bool run = true;
    uint16_t ch;

    noecho();
    curs_set(0);

    getmaxyx(stdscr, max.y, max.x);
    task = newwin(max.y-1, max.x, 0, 0);
    input = newwin(1, max.x, max.y-1, 0);
    getmaxyx(task, max_task.y, max_task.x);
    getmaxyx(input, max_input.y, max_input.x); 
    
    while (run) {
        getmaxyx(stdscr, max.y, max.x);
        wresize(task, max.y-1, max.x);
        wresize(input, 1, max.x);
        mvwin(input, max.y-1, 0);
        getmaxyx(task, max_task.y, max_task.x);
          
        clear();
        wclear(stdscr);
        wclear(task);
        wclear(input);

        box(task, 0, 0);
    
        mvwprintw(task, 0, max_task.x/2-3, "%s", "Tasker");  
        mvwprintw(input, 0, 1, "%s", "Add (A)  Del (D)");
        
        print_table(task);
        
        wrefresh(stdscr);
        wrefresh(task);
        wrefresh(input);

        switch (ch = wgetch(stdscr)) {
            case 'a': case 'A':
                command(input, "add");
                break; 
            case 'd': case 'D':
                command(input, "del");
                break; 
            // case 'c': case 'C': // active
            // case 'u': case 'u': // update
            case 'q': case 'Q':
                run = false;
                break;
        }
    }

    sqlite3_close(tasker.db);
    clear();
    delwin(task);
    endwin();
}

/* Init Ncurses */
void init(void)
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(FALSE);
}

int main(void) {
    init();
    choice(menu());
    task();
    return 0;
}

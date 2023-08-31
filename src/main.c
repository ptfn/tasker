#include <sqlite3.h>
#include <stdbool.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define VERSION "0.1"

typedef struct SQL {
    sqlite3_stmt *stmt;
    sqlite3 *db;
    char *err;
    int rc;
} SQL;

SQL tasker;

/* Window Menu */
uint8_t menu(void)
{
    WINDOW *menu;
    uint16_t ch, yMax, xMax, yMaxM, xMaxM;
    int8_t i = 0;
    char list[3][5] = {"New", "Load", "Exit"};
    char item[5];
        
    getmaxyx(stdscr, yMax, xMax); 
    menu = newwin(5, xMax/4, yMax/2-(5/2), xMax/2-(xMax/4)/2); 
    getmaxyx(menu, yMaxM, xMaxM);
    
    box(menu, 0, 0);            
    
    mvwprintw(menu, 0, xMaxM/2-2, "%s", "Menu"); 
    mvwprintw(stdscr, yMax-1, 0, "Tasker %s", VERSION);
    
    for (i; i < 3; i++) {
        if (i == 0)
            wattron(menu, A_STANDOUT); 
        else
            wattroff(menu, A_STANDOUT);
        sprintf(item, "%-4s", list[i]);
        mvwprintw(menu, i + 1, 2, "* %s", item);
    }
    
    wrefresh(stdscr);
    wrefresh(menu);
    
    noecho();        
    keypad(menu, TRUE); 
    curs_set(0);     
    
    i = 0;

    while ((ch = wgetch(menu)) != 10) {
        sprintf(item, "%-4s", list[i]);
        mvwprintw(menu, i + 1, 2, "* %s", item);
        
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
                delwin(menu);
                endwin();
                exit(0);
        }
        
        wattron(menu, A_STANDOUT);
        sprintf(item, "%-4s", list[i]);
        mvwprintw(menu, i + 1, 2, "- %s", item);
        wattroff(menu, A_STANDOUT);
    }
    
    delwin(menu);
    endwin();

    return i;    
}

/* Function Open or Create DB */
void load(uint8_t menu, char *name_db)
{
    tasker.err = 0;
    tasker.rc = sqlite3_open(name_db, &tasker.db);
        
    if (tasker.rc != SQLITE_OK) {
        sqlite3_close(tasker.db);
    }

    if (!(menu)) {
        char *sql = "DROP TABLE IF EXISTS task;"
                    "CREATE TABLE task(id INTEGER PRIMARY KEY AUTOINCREMENT, task TEXT, time_date REAL);";
        tasker.rc = sqlite3_exec(tasker.db, sql, 0, 0, &tasker.err);
            
        if (tasker.rc != SQLITE_OK) {
            sqlite3_free(tasker.err);
            sqlite3_close(tasker.db);
        }
    }
}

/* Window Input Name DB */
char *open(const char* title)
{
    WINDOW *open;
    uint16_t yMax, xMax, yMaxO, xMaxO;
    char *str = calloc(100, sizeof(char));

    clear();
    getmaxyx(stdscr, yMax, xMax); 
    open = newwin(5, xMax/2, yMax/2-(5/2), xMax/2-(xMax/2)/2); 
    getmaxyx(open, yMaxO, xMaxO);
    
    box(open, 0, 0);            
    
    mvwprintw(open, 0, xMaxO/2-2, "%s", title); 
    mvwprintw(open, yMaxO/2, 2, "%s", "Enter:"); 
    
    wrefresh(stdscr);
    wrefresh(open);

    echo();        
    keypad(open, TRUE); 
    curs_set(1);

    mvwgetstr(open, 2, 9, str);
    delwin(open);
    endwin();

    return str;
}

/* Choice Menu */
void choice(uint8_t menu)
{
    if (menu == 0 || menu == 1) {
        char *path = calloc(100, sizeof(char));
        if (menu)
            path = open("Load");
        else
            path = open("New");

        load(menu, path);
    } else {
        exit(0);
    }
}

/* Print Table On Window */
void print_table(WINDOW *win)
{
    char *sql = "SELECT * FROM task";
    uint16_t i = 1, xMax, yMax;

    tasker.rc = sqlite3_prepare_v2(tasker.db, sql, -1, &tasker.stmt, 0);

    if (tasker.rc != SQLITE_OK) {
        sqlite3_close(tasker.db);
    }

    getmaxyx(win, yMax, xMax);
    mvwprintw(win, 2, 2, "Id  Task");
    mvwprintw(win, 2, xMax-12, "Date");

    while (sqlite3_step(tasker.stmt) == SQLITE_ROW) {
        mvwprintw(win, 3+(i-1), 2, "%d  %s", i++, sqlite3_column_text(tasker.stmt, 1)); 
        mvwprintw(win, 3+(i-1), xMax-12, sqlite3_column_text(tasker.stmt, 2));
    }

    sqlite3_finalize(tasker.stmt);
}

/* Command Window Under SQL */
void command(WINDOW *win, const char *comm)
{
    char *task = calloc(100, sizeof(char));
    char *sql = calloc(200, sizeof(char));
                
    echo();
    curs_set(1);
                
    wclear(win);
    mvwprintw(win, 0, 1, "Enter (%s): ", comm);
    wrefresh(win);
    mvwgetstr(win, 0, 14, task);

    noecho();
    curs_set(0);

    if (!(strcmp(comm, "add")))
        sprintf(sql, "INSERT INTO task (task, time_date) VALUES ('%s', date('now'));", task);
    else if (!(strcmp(comm, "del")))
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
    WINDOW *task, *input;
    bool run = true;
    uint16_t ch, yMax, xMax, yMaxT, xMaxT, yMaxI, xMaxI;

    getmaxyx(stdscr, yMax, xMax);
    task = newwin(yMax-1, xMax, 0, 0);
    input = newwin(1, xMax, yMax-1, 0);
    getmaxyx(task, yMaxT, xMaxT);
    getmaxyx(input, yMaxI, xMaxI); 
    
    while (run) {
        getmaxyx(stdscr, yMax, xMax);
        wresize(task, yMax-1, xMax);
        wresize(input, 1, xMax);
        mvwin(input, yMax-1, 0);
        getmaxyx(task, yMaxT, xMaxT);
          
        clear();
        wclear(stdscr);
        wclear(task);
        wclear(input);

        box(task, 0, 0);
    
        mvwprintw(task, 0, xMaxT/2-3, "%s", "Tasker");  
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
            // case 'c': case 'C':
            // case 'u': case 'u':
            case 'q': case 'Q':
                run = false;
                break;
        }
    }

    sqlite3_close(tasker.db);
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

int main() {
    init();
    choice(menu());
    task();
    return 0;
}

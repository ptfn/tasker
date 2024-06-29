#include "tasker.h"

/* Init Ncurses */
void init(void)
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(FALSE);
    use_default_colors();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    init_pair(3, COLOR_BLACK, COLOR_CYAN);
    init_pair(4, COLOR_BLACK, COLOR_WHITE);
    mousemask(BUTTON1_CLICKED | BUTTON4_PRESSED | BUTTON5_PRESSED, NULL);
}

/* Quit Menu */
// BAG RESIZE WINDOW //
void quit(WINDOW *win_input, bool *run)
{
    int16_t ch; // 16 or 32

    wclear(win_input);
    mvwprintw(win_input, 0, 1, "Quit tasker? [y/N]");
    wrefresh(win_input);

    while ((ch = wgetch(stdscr)) != ENTER_KEY && ch != 'n' && ch != 'N') {
        if (ch == 'y' || ch == 'Y') {
            *run = false; 
            break;
        }
    }
}

/* Print Table On Window */
void print_table(WINDOW *title, WINDOW *main, WINDOW *task, int8_t i, cursor_t *cursor)
{
    // FIXED PRINT UNDER TASK //
    const uint8_t CID = 2, CBAR = 1, CTASK = 7;
    size_t y = 0, t = 0;
    size max_main, max_task;

    wclear(stdscr);
    wclear(title);
    wclear(main);
    box(main, 0, 0);

    getmaxyx(main, max_main.y, max_main.x);
    getmaxyx(task, max_task.y, max_task.x);
    mvwprintw(title, 0, CID, "Id");
    mvwprintw(title, 0, 5, "/");
    mvwprintw(title, 0, CTASK, "Task");

    while (t < tasker->count) {
        size_t x = 0, c = 0, len = strlen(tasker->task[t].name);
        mvwprintw(main, CBAR+y, CID, "%ld", t+1);

        if (cursor->task == t) {
            char buffer_time[12];
            for (uint16_t j = 0; j < tasker->task[cursor->task].count; j++) {
                mvwprintw(task, j+1, 2, "%s", tasker->task[i].under[j].description);
                strftime(buffer_time, 12, "%Y-%m-%d", localtime(&tasker->task[i].under[j].time));
                mvwprintw(task, j+1, max_task.x-12, "%s", buffer_time);
            }
        }

        if (cursor->task == t && !cursor->status) {
            wattron(main, COLOR_PAIR(3));
        }

        while (c < len) {
            if ((CTASK+x) % (max_main.x-2) == 0) {
                x = 0; y++;
            }
            mvwaddch(main, CBAR+y, CTASK+x, tasker->task[t].name[c]);
            x++; c++;
        }
        wattroff(main, COLOR_PAIR(3));
        t++; y++;
    }
    wrefresh(stdscr);
    wrefresh(title);
    wrefresh(main);
}

/* Window Input Path DB */
char *open(const char *title)
{
    char *str = calloc(LEN_FNAME, sizeof(char));
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

        if (strlen(str) != 0)
            break;
    }
    delwin(win_open);
    endwin();

    return str;
}

/* Display Sample Open Window */
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

    while ((ch = wgetch(win_menu)) != ENTER_KEY) {
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

/* Display Sample Menu Window */
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

/* Print Mesage */
void message(const char *message)
{
    size_t msg_len = strlen(message);
    size max, max_msg;
    WINDOW *win_msg;

    getmaxyx(stdscr, max.y, max.x);
    win_msg = newwin(5, max.x/4, max.y/2-(5/2), max.x/2-(max.x/4)/2);
    getmaxyx(win_msg, max_msg.y, max_msg.x);

    box(win_msg, 0, 0);
    mvwprintw(win_msg, max_msg.y/2, max_msg.x/2-(msg_len/2), "%s", message);
    wrefresh(win_msg);

    usleep(DELAY_MSG);
    delwin(win_msg);
    endwin();
}

/* Input Line */
char *input(WINDOW *win, const char *command)
{
    char *task = calloc(SIZE_NAME_TASK, sizeof(char));

    /* Cursor ON */
    echo();
    curs_set(1);
                
    wclear(win);
    mvwprintw(win, 0, 1, "Enter (%s): ", command);
    wrefresh(win);
    mvwgetstr(win, 0, 14, task);

    /* Cursor OFF */
    noecho();
    curs_set(0);
    return task;
}

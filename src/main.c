#include <stdbool.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

/* Macros */
#define VERSION         "0.2.0"
#define SIZE_NAME_TASK  50
#define SIZE_NAME_DESC  100
#define NUM_TASK        64
#define NUM_UNDER       32

/* Struct Tasker DB */
typedef struct task_t
{
    uint16_t count;
    struct task
    {
        char name[SIZE_NAME_TASK];
        uint16_t count;
        struct unde
        {
            char description[SIZE_NAME_DESC];
            time_t time;
            bool active;
        } under[NUM_UNDER];
    } task[NUM_TASK];
} task_t;

/* Struct Size Window */
typedef struct size
{
    uint16_t x, y;
} size;

/* Global Variable (replace?) */
static task_t *tasker;
static FILE *file;

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

    usleep(5e5);
    delwin(win_msg);
    endwin();
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

/* Open File */
FILE *open_file(char *fname, char *mode)
{
    FILE *file;
    if ((file = fopen(fname, mode)) == NULL) {
        perror("Error occured while opening file");
        exit(EXIT_FAILURE);
    }
    return file;
}

/* Create Struct Tasker */
void new(char *path)
{
    file = open_file(path, "wb"); 
    tasker = (task_t*)calloc(1, sizeof(task_t));
    tasker->count = 0;
}

/* Load Struct Tasker */
void load(char *path)
{
    file = open_file(path, "rb+"); 
    tasker = (task_t*)calloc(1, sizeof(task_t));
    fread(tasker, sizeof(task_t), 1, file); 
    rewind(file);
}

/* Display Sample Open Window  */
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

/* Window Input Path DB */
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

        if (strlen(str) != 0)
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
        char *path = calloc(300, sizeof(char));
        if (menu_choice) {
            path = open("Load");
            load(path);
        } else {
            path = open("New");
            new(path);
        }
    } else {
        exit(0);
    }
}

/* Print Table On Window */
void print_table(WINDOW *title, WINDOW *main, WINDOW *task, int8_t i)
{
    // FIXED PRINT UNDER TASK
    const uint8_t CID = 2, CBAR = 1, CTASK = 7;
    size_t y = 0, t = 0;
    size max_main, max_task;

    getmaxyx(main, max_main.y, max_main.x);
    getmaxyx(task, max_task.y, max_task.x);
    mvwprintw(title, 0, CID, "Id");
    mvwprintw(title, 0, 5, "/");
    mvwprintw(title, 0, CTASK, "Task");

    while (t < tasker->count) {
        size_t x = 0, c = 0, len = strlen(tasker->task[t].name);

        if (i == t) {
            wattron(main, COLOR_PAIR(3));
            char buffer_time[12];
            for (uint16_t j = 0; j < tasker->task[i].count; j++) {
                mvwprintw(task, j+1, 2, "%s", tasker->task[i].under[j].description);
                strftime(buffer_time, 12, "%Y-%m-%d", localtime(&tasker->task[i].under[j].time));
                mvwprintw(task, j+1, max_task.x-12, "%s", buffer_time);
            }
        }

        mvwprintw(main, CBAR+y, CID, "%ld", t+1);

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
}

/* Command Window DB */
void command(WINDOW *win, const char *command, int8_t i)
{
    /*
     * ADD DELETE ON I VAR
     * DIVIDE FUNCTION 
     */ 
    char *task = calloc(100, sizeof(char));
                
    echo();
    curs_set(1);
                
    wclear(win);
    mvwprintw(win, 0, 1, "Enter (%s): ", command);
    wrefresh(win);
    mvwgetstr(win, 0, 14, task);

    noecho();
    curs_set(0);

    if (!(strcmp(command, "add"))) {
        if (tasker->count < NUM_TASK) {
            strcpy(tasker->task[tasker->count].name, task);
            tasker->count++;
        }
    } else if (!(strcmp(command, "del"))) {
        // int id_task = atoi(task)-1;
        int id_task = i;
        if (id_task < NUM_TASK && id_task >= 0) {
            memset(&tasker->task[id_task], 0, sizeof(task));
            for (int i = id_task; i < tasker->count-1; i++) {
                struct task *temp = (struct task*)calloc(1, sizeof(struct task)); 
                memcpy(temp, &tasker->task[i+1], sizeof(struct task));
                memcpy(&tasker->task[i+1], &tasker->task[i], sizeof(struct task));
                memcpy(&tasker->task[i], temp, sizeof(struct task));
            }
            tasker->count--;
        }
    } else if (!(strcmp(command, "new"))) {
        if (tasker->task[i].count < NUM_UNDER) {
            strcpy(tasker->task[i].under[tasker->task[i].count].description, task);
            tasker->task[i].under[tasker->task[i].count].time = time(NULL);
            tasker->task[i].count++;
        }
    }
    free(task);
}

/* Main Window */
void task(void)
{
    WINDOW *task, *input, *title, *under;
    size max, max_task;
    bool run = true;
    uint16_t ch;
    int8_t i = 0;

    noecho();
    curs_set(0);

    getmaxyx(stdscr, max.y, max.x);
    task = newwin(max.y-2, max.x/100.0*40, 1, 0);
    under = newwin(max.y-2, max.x/100.0*60, 1, max.x/100.0*40);
    input = newwin(1, max.x, max.y-1, 0);
    title = newwin(1, max.x, 0, 0);
    getmaxyx(task, max_task.y, max_task.x);

    wbkgd(title, COLOR_PAIR(2));
    wbkgd(input, COLOR_PAIR(2));
    wbkgd(task, COLOR_PAIR(1));
    
    while (run) {
        getmaxyx(stdscr, max.y, max.x);
        wresize(title, 1, max.x);
        wresize(task, max.y-2, max.x/100.0*40);
        mvwin(task, 1, 0);
        wresize(under, max.y-2, max.x/100.0*60);
        mvwin(under, 1, max.x/100.0*40);
        wresize(input, 1, max.x);
        mvwin(input, max.y-1, 0);
        getmaxyx(task, max_task.y, max_task.x);
          
        // clear();
        wclear(stdscr);
        wclear(task);
        wclear(input);
        wclear(title);
        wclear(under);

        box(task, 0, 0);
        box(under, 0, 0);

        mvwprintw(input, 0, 1, "%s", "[A] Add  [D] Del  [S] Save  [N] New");
        mvwprintw(title, 0, max.x/100.0*40+2, "Description");
        print_table(title, task, under, i);
        
        wrefresh(stdscr);
        wrefresh(task);
        wrefresh(input);
        wrefresh(title);
        wrefresh(under);

        switch (ch = wgetch(stdscr)) {
            case KEY_UP:
                i--;
                i = (i < 0) ? tasker->count-1 : i;
                break;
            case KEY_DOWN:
                i++;
                i = (i > tasker->count-1) ? 0 : i;
                break;
            case 'a': case 'A':
                command(input, "add", i);
                message("Added task");
                break; 
            case 'n': case 'N':
                command(input, "new", i);
                message("Added under task");
                break;
            case 'd': case 'D':
                command(input, "del", i);
                message("Delete task");
                break; 
            case 's': case 'S':
                fwrite(tasker, sizeof(task_t), 1, file);
                rewind(file);
                message("Save tasks");
                break;
            case 'q': case 'Q':
                run = false;
                break;
        }
    }
    free(tasker);
    fclose(file);

    // clear();
    delwin(task);
    delwin(input);
    delwin(title);
    delwin(under);
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
    use_default_colors();
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_WHITE, COLOR_BLUE);
    init_pair(3, COLOR_BLACK, COLOR_CYAN);
}

int main(void) {
    init();
    choice(menu());
    task();
    return 0;
}

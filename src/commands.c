#include "tasker.h"

/* Global Variable */
task_t *tasker;
FILE *file;

/* Linked Functon Menu */
void choice(uint8_t menu_choice)
{
    char *path = calloc(LEN_PATH, sizeof(char));
    if (menu_choice != 2) {
        if (menu_choice) {
            path = open("Load");
            load_file(path);
        } else {
            path = open("New");
            new_file(path);
        }
    } else {
        exit(0);
    }
}

/* Create New Struct Tasker */
void new_file(char *path)
{
    file = open_file(path, "wb"); 
    tasker = (task_t*)calloc(1, sizeof(task_t));
    tasker->count = 0;
    free(path);
}

/* Load Struct Tasker */
void load_file(char *path)
{
    file = open_file(path, "rb+"); 
    tasker = (task_t*)calloc(1, sizeof(task_t));
    fread(tasker, sizeof(task_t), 1, file); 
    rewind(file);
    free(path);
}

/* Add Item */
void add_task(char *text, int8_t i) // int8_t normally? wtf not get arg
{
    if (tasker->count < NUM_TASK) {
        strcpy(tasker->task[tasker->count].name, text);
        tasker->count++;
    }
    free(text);
}

/* Delete Item */
void del_task(int8_t i)
{
    int id_task = i;
    if (i < NUM_TASK && i >= 0) {
        memset(&tasker->task[i], 0, sizeof(task));
        for (int j = i; j < tasker->count-1; j++) {
            struct task *temp = (struct task*)calloc(1, sizeof(struct task)); 
            // MAYBE SEPARATE FUNCTION
            memcpy(temp, &tasker->task[j+1], sizeof(struct task));
            memcpy(&tasker->task[j+1], &tasker->task[j], sizeof(struct task));
            memcpy(&tasker->task[j], temp, sizeof(struct task));
            free(temp);
        }
        tasker->count--;
    }
}

/* Add Under Task */
void new_under(char *text, int8_t i)
{
    if (tasker->task[i].count < NUM_UNDER) {
        strcpy(tasker->task[i].under[tasker->task[i].count].description, text);
        tasker->task[i].under[tasker->task[i].count].time = time(NULL);
        tasker->task[i].count++;
    }
    free(text);
}

/* Command Tasker */
void command(enum keys key, WINDOW *win_input, int8_t *i, bool *run)
{
    switch (key) {
        case ADD:
            add_task(input(win_input, "add"), *i);
            message("Added task");
            break;
        case DEL:
            del_task(*i);
            message("Delete task");
            break;
        case SAVE:
            fwrite(tasker, sizeof(task_t), 1, file);
            rewind(file);
            message("Save tasks");
            break;
        case NEW:
            new_under(input(win_input, "new"), *i);
            message("Added under task");
            break;
        case EXIT:
            quit(win_input, run);
            break;
        case UP:
            *i = *i - 1;
            *i = (*i < 0) ? tasker->count-1 : *i;
            break;
        case DWN:
            *i = *i + 1;
            *i = (*i > tasker->count-1) ? 0 : *i;
            break;
   } 
}

/* Main Window */
void task(void)
{
    /* Init Variable */
    WINDOW *task, *win_input, *title, *under;
    size max_std, max_new;
    bool run = true;
    int8_t i = 0;
    uint16_t ch;
    MEVENT ms;

    noecho();
    curs_set(0);

    /* Init Windows */
    getmaxyx(stdscr, max_std.y, max_std.x);
    task = newwin(max_std.y-2, round(max_std.x/100.0*PERC_TASK), 1, 0);
    under = newwin(max_std.y-2, round(max_std.x/100.0*PERC_UNDER), 1, round(max_std.x/100.0*PERC_TASK));
    win_input = newwin(1, max_std.x, max_std.y-1, 0);
    title = newwin(1, max_std.x, 0, 0);

    /* Setting Up Color Pair
     * Choose background and foreuground color
     */
    wbkgd(title, COLOR_PAIR(2));
    wbkgd(win_input, COLOR_PAIR(2));
    wbkgd(task, COLOR_PAIR(1));
    
    while (run) {
        getmaxyx(stdscr, max_new.y, max_new.x);
        if (max_new.x != max_std.x || max_new.y != max_std.y) {
            getmaxyx(stdscr, max_std.y, max_std.x);
            wresize(title, 1, max_std.x);
            wresize(task, max_std.y-2, round(max_std.x/100.0*40.0));
            mvwin(task, 1, 0);
            wresize(under, max_std.y-2, round(max_std.x/100.0*60.0));
            mvwin(under, 1, round(max_std.x/100.0*40.0));
            wresize(win_input, 1, max_std.x);
            mvwin(win_input, max_std.y-1, 0);
        }

        wclear(task);
        wclear(win_input);
        wclear(title);
        wclear(under);

        box(task, 0, 0);
        box(under, 0, 0);

        mvwprintw(win_input, 0, 1, "%s", "[A] Add  [D] Del  [S] Save  [N] New");
        mvwprintw(title, 0, round(max_std.x/100.0*PERC_TASK)+2, "Description");
        mvwprintw(title, 0, max_std.x-12, "Date");
        print_table(title, task, under, i);

        wrefresh(task);
        wrefresh(win_input);
        wrefresh(title);
        wrefresh(under);
        
        switch (ch = wgetch(stdscr)) {
            // Mouse controle
            case KEY_MOUSE:
                if (getmouse(&ms) == OK) {
                    if (ms.bstate & BUTTON4_PRESSED)
                        command(UP, win_input, &i, &run);
                    else if (ms.bstate & BUTTON5_PRESSED)
                        command(DWN, win_input, &i, &run);
                    else if (ms.bstate & BUTTON1_CLICKED)
                        if (ms.y == max_std.y-1) {
                            /* Commands Panel */
                            if (ms.x > 0 && ms.x < 8)
                                command(ADD, win_input, &i, &run);
                            else if (ms.x > 9 && ms.x < 17)
                                command(DEL, win_input, &i, &run);
                            else if (ms.x > 18 && ms.x < 27)
                                command(SAVE, win_input, &i, &run);
                            else if (ms.x > 28 && ms.x < 36)
                                command(NEW, win_input, &i, &run);
                        }
                } break;

            // Keyboard controle
            case KEY_UP:
                command(UP, win_input, &i, &run);
                break;
            case KEY_DOWN:
                command(DWN, win_input, &i, &run);
                break;
            case 'a': case 'A':
                command(ADD, win_input, &i, &run);
                break;
            case 'n': case 'N':
                command(NEW, win_input, &i, &run);
                break;
            case 'd': case 'D':
                command(DEL, win_input, &i, &run);
                break;
            case 's': case 'S':
                command(SAVE, win_input, &i, &run);
                break;
            case 'q': case 'Q':
                command(EXIT, win_input, &i, &run);
                break;
        }
    }
    free(tasker);
    fclose(file);

    delwin(task);
    delwin(win_input);
    delwin(title);
    delwin(under);
    clear();
    endwin();
}

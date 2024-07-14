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
void add_task(char *text)
{
    if (tasker->count < NUM_TASK) {
        strcpy(tasker->task[tasker->count].name, text);
        tasker->count++;
    }
    free(text);
}

/* Delete Item */
void del_task(cursor_t *cursor)
{
    int id_task = cursor->task;
    if (id_task < NUM_TASK && id_task >= 0) { // why? cursor?
        if (!cursor->status) {
            // method zero under task
            memset(&tasker->task[cursor->task], 0, sizeof(struct task)); // why memset?
            for (int j = cursor->task; j < tasker->count-1; j++) {
                swap(&tasker->task[j+1], &tasker->task[j], sizeof(struct task));
            }
            tasker->count--;
        } else {
            memset(&tasker->task[cursor->task].under[cursor->under], 0, sizeof(struct under));
            for (int j = cursor->under; j < tasker->task[cursor->task].count-1; j++) {
                swap(&tasker->task[cursor->task].under[j+1], 
                     &tasker->task[cursor->task].under[j], 
                     sizeof(struct under));
            }
            tasker->task[cursor->task].count--;
        }
    }
}

/* Add Under Task */
void new_under(char *text, cursor_t *cursor)
{
    if (tasker->task[cursor->task].count < NUM_UNDER) {
        strcpy(tasker->task[cursor->task].under[tasker->task[cursor->task].count].description, text);
        tasker->task[cursor->task].under[tasker->task[cursor->task].count].time = time(NULL);
        tasker->task[cursor->task].under[tasker->task[cursor->task].count].status = 0;
        tasker->task[cursor->task].count++;
    }
    free(text);
}

void update_task(char *text, cursor_t *cursor)
{
    if (!cursor->status) {
        if (cursor->task < NUM_TASK)
            strcpy(tasker->task[cursor->task].name, text);
    } else {
        if (cursor->under < NUM_UNDER)
            strcpy(tasker->task[cursor->task].under[cursor->under].description, text);
    }
    free(text);
}

void change_status(cursor_t *cursor)
{
    uint8_t *status = &tasker->task[cursor->task].under[cursor->under].status;
    if (cursor->status) {
        *status = *status + 1; 
        *status = (*status > 2) ? 0 : *status; 
    }
}

/* Command Tasker */
void command(enum keys key, WINDOW *win_input, cursor_t *cursor, bool *run)
{
    // REPLACE ARGS "i" to CURSOR AND UNDER ELEMENT STRUCT //
    // READY? //
    switch (key) {
        case ADD:
            add_task(input(win_input, "add"));
            message("Added task");
            break;
        case DEL:
            del_task(cursor);
            message("Delete task");
            break;
        case UPD:
            update_task(input(win_input, "upd"), cursor);
            message("Update task");
            break;
        case CHN:
            change_status(cursor);
            break;
        case SAVE:
            fwrite(tasker, sizeof(task_t), 1, file);
            rewind(file);
            message("Save tasks");
            break;
        case NEW:
            new_under(input(win_input, "new"), cursor);
            message("Added under task");
            break;
        case EXIT:
            quit(win_input, run);
            break;
        case UP:
            if (cursor->status) {
                cursor->under = cursor->under - 1;
                cursor->under = (cursor->under < 0) ? tasker->task[cursor->task].count-1 : cursor->under;
            } else {
                cursor->under = 0;
                cursor->task = cursor->task - 1;
                cursor->task = (cursor->task < 0) ? tasker->count-1 : cursor->task;
            }
            break;
        case DWN:
            if (cursor->status) {
                cursor->under = cursor->under + 1;
                cursor->under = (cursor->under > tasker->task[cursor->task].count-1) ? 0 : cursor->under;
            } else {
                cursor->under = 0;
                cursor->task = cursor->task + 1;
                cursor->task = (cursor->task > tasker->count-1) ? 0 : cursor->task;
            }
            break;
        case TAB:
            cursor->status = cursor->status ? 0 : 1;
            break;
    } 
}

/* Main Window */
void task(void)
{
    /* Init Variable */
    WINDOW *task, *win_input, *title, *under;
    size max_std, max_new;
    cursor_t *cursor = calloc(1, sizeof(cursor_t));
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
            wresize(task, max_std.y-2, round(max_std.x/100.0*PERC_TASK));
            mvwin(task, 1, 0);
            wresize(under, max_std.y-2, round(max_std.x/100.0*PERC_UNDER));
            mvwin(under, 1, round(max_std.x/100.0*PERC_TASK));
            wresize(win_input, 1, max_std.x);
            mvwin(win_input, max_std.y-1, 0);
        }

        wclear(task);
        wclear(win_input);
        wclear(title);
        wclear(under);

        box(task, 0, 0);
        box(under, 0, 0);

        mvwprintw(win_input, 0, 1, "%s", "[A] Add  [D] Del  [S] Save  [N] New  [U] Update [C] Change");
        print_table(title, task, under, i, cursor);

        wrefresh(task);
        wrefresh(win_input);
        wrefresh(title);
        wrefresh(under);
        
        switch (ch = wgetch(stdscr)) {
            // Mouse controle
            case KEY_MOUSE:
                if (getmouse(&ms) == OK) {
                    if (ms.bstate & BUTTON4_PRESSED)
                        command(UP, win_input, cursor, &run);
                    else if (ms.bstate & BUTTON5_PRESSED)
                        command(DWN, win_input, cursor, &run);
                    else if (ms.bstate & BUTTON1_CLICKED)
                        if (ms.y == max_std.y-1) {
                            /* Commands Panel */
                            if (ms.x > 0 && ms.x < 8)
                                command(ADD, win_input, cursor, &run);
                            else if (ms.x > 9 && ms.x < 17)
                                command(DEL, win_input, cursor, &run);
                            else if (ms.x > 18 && ms.x < 27)
                                command(SAVE, win_input, cursor, &run);
                            else if (ms.x > 28 && ms.x < 36)
                                command(NEW, win_input, cursor, &run);
                            // add new buttons
                        }
                } break;

            // Keyboard controle
            case KEY_UP:
                command(UP, win_input, cursor, &run);
                break;
            case KEY_DOWN:
                command(DWN, win_input, cursor, &run);
                break;
            case TAB_KEY: case KEY_LEFT: case KEY_RIGHT:
                command(TAB, win_input, cursor, &run);
                break;
            case 'a': case 'A':
                command(ADD, win_input, cursor, &run);
                break;
            case 'n': case 'N':
                command(NEW, win_input, cursor, &run);
                break;
            case 'd': case 'D':
                command(DEL, win_input, cursor, &run);
                break;
            case 's': case 'S':
                command(SAVE, win_input, cursor, &run);
                break;
            case 'u': case 'U':
                command(UPD, win_input, cursor, &run);
                break;
            case 'c': case 'C':
                command(CHN, win_input, cursor, &run);
                break;
            case 'q': case 'Q':
                command(EXIT, win_input, cursor, &run);
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

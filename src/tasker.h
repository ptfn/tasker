#ifndef _TASKERLIB_H_
#define _TASKERLIB_H_

/* Libraries */
#include <stdbool.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

/* Macros */
#define VERSION         "0.3.0"
#define SIZE_NAME_TASK  50
#define SIZE_NAME_DESC  100
#define NUM_TASK        64
#define NUM_UNDER       32
#define PERC_TASK       40.0
#define PERC_UNDER      60.0
#define LEN_PATH        4096
#define LEN_FNAME       255
#define DELAY_MSG       5e5
#define ENTER_KEY       10
#define TAB_KEY         9

/* Struct Tasker DB */
typedef struct task_t
{
    uint16_t count;
    struct task
    {
        char name[SIZE_NAME_TASK];
        uint16_t count;
        struct under
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

typedef struct cursor_t
{
    // not signed
    int16_t task;
    int16_t under;
    bool status;
} cursor_t;

/* Enum Commands Tasker */
enum keys {ADD, DEL, SAVE, NEW, EXIT, UP, DWN, TAB};

/* Extern Global Variable */
extern task_t *tasker;
extern FILE *file;

/* Prototype Functon */
void message(const char *message);
void display_menu(WINDOW *win_menu, uint16_t xMaxM,
                  uint16_t yMax, int8_t i, 
                  char *item, char list[3][5]);
uint8_t menu(void);
FILE *open_file(char *fname, char *mode);
void new_file(char *path);
void load_file(char *path);
void display_open(WINDOW *win_open, size max_open, const char *title);
char *open(const char *title);
void choice(uint8_t menu_choice);
void print_table(WINDOW *title, WINDOW *main, WINDOW *task, int8_t i, cursor_t *cursor);
char *input(WINDOW *win, const char *command);
void quit(WINDOW *win_input, bool *run);
void command(enum keys key, WINDOW *win_input, cursor_t *cursor, bool *run);
void add_task(char *text, int8_t i);
void del_task(int8_t i);
void new_under(char *text, int8_t i);
void task(void);
void init(void);

#endif

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void play(void);
int **init_field(int const HIGH, int const WIDTH);
int **init(int const HIGH, int const WIDTH);
int **alloc_mem(int n, int m);
void free_mem(int **arr, int n);
void print_field(int **field, int n, int m);
void init_borders(int **field, int n, int m);
void fill_space(int **field, int n, int m);
void future_field_update(int **currentState, int **futureState, int n, int m);
int life_count(int **currentState, int row, int column);
int annihilation_check(int **futureState, int n, int m);
int close_repeat_check(int **currentState, int **futureState, int **pastState, int n, int m);
void copy(int **toMatrix, int **fromMatrix, int n, int m);
void read_pattern(int **arr, int n,
                  int m);  // load pattern from std input; "a.out < pattern.txt"
void switcher(char, long int *, int *);

int main(void) {
    if (isatty(0) == 1) {  // checking for empty redirected stdin
        printf("No pattern file selected");
    } else {
        play();
    }

    return (0);
}

void play(void) {
    int const WIDTH = 80;
    int const HIGH = 25;
    int **field = init_field(HIGH, WIDTH);
    int **future = init_field(HIGH, WIDTH);
    int **past = init_field(HIGH, WIDTH);
    int liveFlag = 0, repeatFlag = 0;
    read_pattern(field, HIGH, WIDTH);
    if (freopen("/dev/tty", "r", stdin)) {
        initscr();
        nodelay(stdscr, true);
        noecho();
        curs_set(0);
        timeout(50);
    }
    long int time_ms = 1000;
    int exit_flag = 0;
    while (liveFlag != 1 && exit_flag != 1 && repeatFlag != 1794) {
        wclear(stdscr);
        print_field(field, HIGH, WIDTH);  // print initial state
        future_field_update(field, future, HIGH, WIDTH);
        liveFlag = annihilation_check(future, HIGH, WIDTH);
        repeatFlag = close_repeat_check(field, future, past, HIGH, WIDTH);
        copy(past, field, HIGH, WIDTH);
        copy(field, future, HIGH, WIDTH);
        char button = getch();
        switcher(button, &time_ms, &exit_flag);
        usleep(time_ms * 100);
    }
    wclear(stdscr);
    print_field(field, HIGH, WIDTH);
    printw("\t\t\tThat's it, come again!");
    refresh();
    free_mem(field, HIGH);
    free_mem(future, HIGH);
    free_mem(past, HIGH);
}

int **init_field(int const HIGH, int const WIDTH) {
    int **field;
    field = alloc_mem(HIGH, WIDTH);
    fill_space(field, HIGH, WIDTH);  // fill with white spaces
    init_borders(field, HIGH, WIDTH);

    return (field);
}

int **alloc_mem(int n, int m) {
    int **arr = (int **)malloc(n * sizeof(int *));
    for (int row = 0; row < n; row++) {
        /* rows pointers*/
        arr[row] = malloc(m * sizeof(int));
    }

    return (arr);
}

void free_mem(int **arr, int n) {
    for (int row = 0; row < n; row++) {
        free(arr[row]);
    }
    free(arr);
}

void print_field(int **field, int n, int m) {
    char ch;
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            if (field[row][col] == 1) {
                ch = 'x';
            } else if (field[row][col] == 8) {
                ch = '=';
            } else if (field[row][col] == 7) {
                ch = '|';
            } else {
                ch = ' ';
            }
            printw("%c", ch);
        }
        printw("\n");
    }
}

void init_borders(int **field, int n, int m) {
    /* Write border symbols to fileld array
    ** n - high (oy), m - width (ox).
    */
    int const hor_sym = 8;
    char const vert_sym = 7;
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            if ((row == 0 || row == n - 1) && (col != 0 || col != m - 1)) {
                field[row][col] = hor_sym;
            }
            if ((col == 0 || col == m - 1) && (row != 0 && row != n - 1)) {
                field[row][col] = vert_sym;
            }
        }
    }
}

void future_field_update(int **currentState, int **futureState, int n, int m) {
    for (int row = 1; row < n - 1; row++) {
        for (int col = 1; col < m - 1; col++) {
            futureState[row][col] = life_count(currentState, row, col);
            if (currentState[row][col] == 1 && (futureState[row][col] == 2 || futureState[row][col] == 3))
                futureState[row][col] = 1;
            else if ((currentState[row][col] == 0) && (futureState[row][col] == 3))
                futureState[row][col] = 1;
            else
                futureState[row][col] = 0;
        }
    }
}

void fill_space(int **field, int n, int m) {
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            field[row][col] = 0;
        }
    }
}

int annihilation_check(int **futureState, int n, int m) {
    int live = 1;
    for (int row = 1; row < n - 1; row++) {
        for (int col = 1; col < m - 1; col++) {
            if (futureState[row][col] == 1) live = 0;
        }
    }
    return live;
}

int close_repeat_check(int **currentState, int **futureState, int **pastState, int n, int m) {
    int repeat = 0;
    for (int row = 1; row < n - 1; row++) {
        for (int col = 1; col < m - 1; col++) {
            if (pastState[row][col] == currentState[row][col] || pastState[row][col] == futureState[row][col])
                repeat++;
        }
    }
    return repeat;
}

void copy(int **toMatrix, int **fromMatrix, int n, int m) {
    for (int row = 0; row < n; row++) {
        for (int col = 0; col < m; col++) {
            toMatrix[row][col] = fromMatrix[row][col];
        }
    }
}

void switcher(char button, long int *speed, int *exit_flag) {
    switch (button) {
        case '+': {
            *speed -= 100;
            if (*speed < 100) {
                *speed = 100;
            }
        } break;
        case '-': {
            *speed += 100;
            if (*speed > 10000) {
                *speed = 10000;
            }
        } break;
        case 'q':
        case 'Q':
            *exit_flag = 1;
            break;
    }
}

int life_count(int **currentState, int row, int column) {
    int count = 0, cur_row = 0, cur_col = 0, xBorderCrossFlag, yBorderCrossFlag;
    for (int i_dx = -1; i_dx < 2; i_dx++)
        for (int j_dx = -1; j_dx < 2; j_dx++) {
            xBorderCrossFlag = 0;
            yBorderCrossFlag = 0;
            if ((row + i_dx) < 1) {
                cur_row = 23;
                xBorderCrossFlag = 1;
            }
            if ((row + i_dx) > 23) {
                cur_row = 1;
                xBorderCrossFlag = 1;
            }
            if ((column + j_dx) < 1) {
                cur_col = 78;
                yBorderCrossFlag = 1;
            }
            if ((column + j_dx) > 78) {
                cur_col = 1;
                yBorderCrossFlag = 1;
            }

            if (((xBorderCrossFlag == 0 && yBorderCrossFlag == 0) &&
                 currentState[(row + i_dx)][(column + j_dx)] == 1) ||
                ((xBorderCrossFlag == 1 && yBorderCrossFlag == 0) &&
                 currentState[cur_row][((column + j_dx))] == 1) ||
                ((xBorderCrossFlag == 0 && yBorderCrossFlag == 1) &&
                 currentState[(row + i_dx)][cur_col] == 1) ||
                ((xBorderCrossFlag == 1 && yBorderCrossFlag == 1) && currentState[cur_row][cur_col] == 1)) {
                count++;
            }
        }
    if (count > 1 && (currentState[row][column] != 0)) {
        count--;
    }
    return count;
}

void read_pattern(int **arr, int n, int m) {
    char ch;
    for (int row = 1; row < (n - 1); row++) {
        for (int col = 1; col < (m - 1); col++) {
            ch = getc(stdin);
            if (ch == '\n' || ch == -1) {
                break;
            }
            if (ch == '0') {
                arr[row][col] = 0;
            } else
                arr[row][col] = 1;
        }
    }
}
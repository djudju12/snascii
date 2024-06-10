#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <time.h>

#define WIDTH  20
#define HEIGTH WIDTH/2

int grid[HEIGTH][WIDTH];

float dt;

#define IS_WALL(x, y) (x == 0 || x == (WIDTH - 1) || y == 0 || y == (HEIGTH - 1))

void init_grid(void) {
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            grid[y][x] = IS_WALL(x, y) ? 1 : 0 ;
        }
    }
}

const char* grid_table = " #";
void draw_grid(void) {
    if (!should_draw()) return;
    printf("\033[0;1f\n");
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            printf("%c", grid_table[grid[y][x]]);
        }
        printf("\n");
    }
}

int should_draw(void) {
    return dt > 1/60.0;
}

struct termios old_term;

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term);
}

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &old_term);
    atexit(disable_raw_mode);

    struct termios raw = old_term;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw);
    fcntl(STDOUT_FILENO, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
}

double get_time_ms(void) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec*1e3 + spec.tv_nsec/1.0e6;
}

void update(int* should_stop) {
    char c;
    read(STDIN_FILENO, &c, 1);
    switch (c) {
        case 'w': {} break;

        case 'a': {} break;

        case 'd': {} break;

        case 's': {} break;

        case 'q': {
            *should_stop = 1;
        } break;

        default: {} break;
    }
}

int main(void) {
    double last_frame = get_time_ms();
    while (1) {
        if (last_frame - get_time_ms() > 1/60.0) {
            printf("hello\n");
        }
    }
    return 0;
    int should_stop = 0;
    enable_raw_mode();
    init_grid();
    while (!should_stop) {
        update(&should_stop);
        draw_grid();
    }
}
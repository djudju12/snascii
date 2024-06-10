#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

// #include <fcntl.h>
#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>

#include <time.h>

#define WIDTH  20
#define HEIGTH WIDTH/2

#define IS_WALL(x, y) (x == 0 || x == (WIDTH - 1) || y == 0 || y == (HEIGTH - 1))

struct Game {
    float last_frame;
    int should_stop;
    int grid[HEIGTH][WIDTH];
    struct pollfd fds;
};

struct Game game = {0};

double get_time_ms(void) {
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    return spec.tv_sec + spec.tv_nsec/1.0e6;
}

double get_dt_ms(void) {
    return get_time_ms() - game.last_frame;
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

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int has_input(void) {
    return poll(&game.fds, 1, 0);
}

void update() {
    char c;
    if (has_input()) {
        read(STDIN_FILENO, &c, 1);
        switch (c) {
            case 'w': {} break;
            case 'a': {} break;
            case 'd': {} break;
            case 's': {} break;
            case 'q': { game.should_stop = 1; } break;
            default: {} break;
        }
    }
}

int should_draw(void) {
    if (get_dt_ms() >= 1/60.0) {
        game.last_frame = get_time_ms();
        return 1;
    }

    return 0;
}

void draw_grid(void) {
    static const char* grid_table = " #";
    printf("\033[0;0f");
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            printf("%c", grid_table[game.grid[y][x]]);
        }
        printf("\n");
    }
}

void init_grid(void) {
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            game.grid[y][x] = IS_WALL(x, y) ? 1 : 0 ;
        }
    }
}

void clear(void) {
    printf("\033[2J");
}

void init(void) {
    game.last_frame = get_time_ms();
    game.should_stop = 0;
    game.fds.fd = STDIN_FILENO;
    game.fds.events = POLLIN;
    init_grid();
    enable_raw_mode();
}

int main(void) {
    init();
    clear();
    while (!game.should_stop) {
        update();
        draw_grid();
    }
}
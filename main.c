#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

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
    Vec2 apple;
};

typedef enum {
    UP     = 'w',
    DOWN   = 's',
    LEFT   = 'a',
    RIGHT  = 'd'
} Direction;

typedef struct {
    int x, y;
} Vec2;

#define MAX_SIZE 20
struct Snake {
    int size;
    Vec2 body[MAX_SIZE];
    Direction direction;
};

struct Game game   = {0};
struct Snake snake = {0};

double get_time_sec(void) {
    struct timespec spec;
    if (clock_gettime(CLOCK_MONOTONIC, &spec) == -1) exit(1);
    return spec.tv_sec + spec.tv_nsec/1e9;
}

double get_delta_time(void) {
    return get_time_sec() - game.last_frame;
}

struct termios old_term;

void disable_raw_mode(void) {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_term);
    printf("\e[?25h");
}

void enable_raw_mode(void) {
    tcgetattr(STDIN_FILENO, &old_term);
    atexit(disable_raw_mode);

    struct termios raw = old_term;
    raw.c_lflag &= ~(ECHO | ICANON);

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    printf("\e[?25l");
    printf("\033[2J");
}

int has_input(void) {
    return poll(&game.fds, 1, 0);
}

void update() {
    char c;
    if (has_input()) {
        read(STDIN_FILENO, &c, 1);
        switch (c) {
            case UP: case DOWN: case LEFT: case RIGHT: {
                snake.direction = c;
            } break;

            case 'q': {
                game.should_stop = 1;
            } break;

            default: {} break;
        }
    }

    for (int i = 0; i < snake.size; i++) {
        Vec2 pos = snake.body[i];
        game.grid[pos.y][pos.x] = 2;
    }

    float dt = get_delta_time();
    if (dt >= 1/8.0) {
        Vec2 head = snake.body[0];
        if (game.grid[head.y][head.y] == '*') {
            snake.size += 1;
            if (snake.size == MAX_SIZE) exit(1);
            for (int i = snake.size; i >= 1; i++) {
                snake.body[i + 1] = snake.body[i];
            }
        } else {
            for (int i = 0; i < snake.size; i++) {
                snake.body[i + 1] = snake.body[i];
            }
        }

        game.last_frame = get_time_sec();
        switch (snake.direction) {
            case UP: {
                head.y -= 1;
            } break;

            case DOWN: {
                head.y += 1;
            } break;

            case LEFT: {
                head.x -= 2;
            } break;

            case RIGHT: {
                head.x += 2;
            } break;

            default: {} break;
        }

        if      (head.x >= (WIDTH - 1)) head.x = 1;
        else if (head.x <= 0)           head.x = WIDTH - 2;

        if      (head.y == (HEIGTH - 1)) head.y = 1;
        else if (head.y == 0)            head.y = HEIGTH - 2;

        snake.body[0] = head;
    }
}

void clear(void) {
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            game.grid[y][x] = IS_WALL(x, y) ? 1 : 0;
        }
    }
}

static const char* grid_table = " #o*";
void draw_grid(void) {
    printf("\033[0;0f");
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            printf("%c", grid_table[game.grid[y][x]]);
        }
        printf("\n");
    }

    printf("score: %2d\n", snake.size - 1);
}

int main(void) {
    game.should_stop = 0;
    game.fds.fd = STDIN_FILENO;
    game.fds.events = POLLIN;

    snake.body[0].x = WIDTH/2;
    snake.body[0].y = HEIGTH/2;

    snake.body[1].x = WIDTH/2;
    snake.body[1].y = HEIGTH/2 + 1;

    snake.size = 2;
    snake.direction = UP;

    enable_raw_mode();
    game.last_frame = get_time_sec();
    while (!game.should_stop) {
        update();
        draw_grid();
        clear();
    }
}
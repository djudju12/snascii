#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#include <math.h>

#include <sys/poll.h>
#include <termios.h>
#include <unistd.h>

#include <time.h>

#define WIDTH  20
#define HEIGTH WIDTH/2
#define IS_WALL(x, y) (x == 0 || x == (WIDTH - 1) || y == 0 || y == (HEIGTH - 1))

typedef struct {
    float x, y;
} Vec2;

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

enum Objects {
    BLANK_SPACE = 0,
    WALL,
    SNAKE,
    APPLE
};

typedef struct Body_Part Body_Part;

struct Body_Part {
    Vec2 pos;
    Body_Part* next;
    Body_Part* prev;
};

#define MAX_SIZE 20

Body_Part body_part_poll[MAX_SIZE];

struct Snake {
    int size;
    Body_Part* head;
    Body_Part* tail;
    Direction direction;
    Vec2 vel;
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

void generate_apple(void) {
    game.apple.x += 1;
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

    for (Body_Part* body = snake.head; body != NULL; body = body->next) {
        Vec2 pos = body->pos;
        int x = pos.x;
        int y = pos.y;
        game.grid[y][x] = SNAKE;
    }

    game.grid[(int) game.apple.y][(int) game.apple.x] = APPLE;

    double dt = get_delta_time();
    game.last_frame = get_time_sec();

    Vec2 next_move = snake.head->pos;
    switch (snake.direction) {
        case UP: {
            next_move.y = next_move.y - dt*snake.vel.y;
        } break;

        case DOWN: {
            next_move.y = next_move.y + dt*snake.vel.y;
        } break;

        case LEFT: {
            next_move.x = next_move.x - dt*snake.vel.x;
        } break;

        case RIGHT: {
            next_move.x = next_move.x + dt*snake.vel.x;
        } break;

        default: {} break;
    }

    int nx = next_move.x;
    int ny = next_move.y;

    int move = nx != (int) snake.head->pos.x || ny != (int) snake.head->pos.y;
    if (move) {

        if      (nx == (WIDTH - 1))  nx = 1;
        else if (nx == 0)            nx = WIDTH - 2;

        if      (ny == (HEIGTH - 1)) ny = 1;
        else if (ny == 0)            ny = HEIGTH - 2;

        Body_Part* new_head = NULL;
        if (game.grid[ny][nx] == APPLE) {
            if (snake.size >= MAX_SIZE - 1) {
                game.should_stop = 1;
            } else {
                new_head = &body_part_poll[snake.size];
                snake.size += 1;
                new_head->next = snake.head;
                new_head->prev = NULL;
                snake.head->prev = new_head;
                snake.head = new_head;
                if (snake.size == 2) {
                    snake.tail = snake.head->next;
                }
                generate_apple();
            }
        } else {
            new_head = snake.tail;
            if (new_head != NULL) {
                snake.head->prev = new_head;
                new_head->next = snake.head;
                snake.head = new_head;

                snake.tail = new_head->prev;
                snake.tail->next = NULL;
                snake.head->prev = NULL;
            }
        }
    }

    snake.head->pos.x = next_move.x;
    snake.head->pos.y = next_move.y;
    usleep(1000);
}

void clear(void) {
    for (size_t y = 0; y < HEIGTH; y++) {
        for (size_t x = 0; x < WIDTH; x++) {
            game.grid[y][x] = IS_WALL(x, y) ? WALL : BLANK_SPACE;
        }
    }
}

static const char* grid_table = " #O*";
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

    game.apple.x = 1;
    game.apple.y = HEIGTH / 2;

    snake.head = &body_part_poll[snake.size++];
    snake.head->pos.x = WIDTH/2;
    snake.head->pos.y = HEIGTH/2;
    snake.head->next = NULL;
    snake.head->prev = NULL;
    snake.tail = NULL;

    snake.vel.x = 7;
    snake.vel.y = 5;
    snake.direction = UP;

    enable_raw_mode();
    game.last_frame = get_time_sec();
    while (!game.should_stop) {
        clear();
        update();
        draw_grid();
    }
}
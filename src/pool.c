#include <raylib.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "raymath.h"
#include <math.h>

#define WIDTH 800
#define HEIGHT 800
#define RADIUS 7
#define FPS 60 
#define PERCENT_ENERGY_LEFT_ON_HIT 75

#define da_append(xs, x)                                                             \
    do {                                                                             \
        if ((xs)->count >= (xs)->capacity) {                                         \
            if ((xs)->capacity == 0) (xs)->capacity = 256;                           \
            else (xs)->capacity *= 2;                                                \
            (xs)->items = realloc((xs)->items, (xs)->capacity*sizeof(*(xs)->items)); \
        }                                                                            \
                                                                                     \
        (xs)->items[(xs)->count++] = (x);                                            \
    } while (0)

typedef struct {
    Vector2 *items;
    size_t capacity;
    size_t count;
} Trace;

typedef struct {
    Vector2 coords;
    Vector2 velocity;
    Trace trace;
    Color color;
} Ball;

typedef struct {
    Ball *items;
    size_t capacity;
    size_t count;
} Balls;

typedef struct {
    Vector2 *items;
    size_t capacity;
    size_t count;
} Poly;

typedef struct {
    Vector2 *items;
    size_t capacity;
    size_t count;
} PointsTrace;

void moveBall(Ball *ball)
{
    ball->coords.x += ball->velocity.x * 1/FPS;
    ball->coords.y += ball->velocity.y * 1/FPS;
}

int randomValue(int rand_max)
{
    return rand() % (2 * rand_max) - rand_max; 
}

Vector2 calculate_ball_reflect(Vector2 point1, Vector2 point2, Vector2 ball_velocity)
{
    Vector2 normal = Vector2Subtract(point2, point1);
    Vector2 perp_vector = Vector2Normalize((Vector2){ -normal.y, normal.x });
    Vector2 reflect = Vector2Reflect(ball_velocity, perp_vector);

    return reflect;
}

int main(void)
{
    srand(time(NULL));
    Balls balls = {0};
    Poly poly = {0};
    InitWindow(WIDTH, HEIGHT, "raylib example - basic window");
    SetTargetFPS(FPS);
    bool drawTrace = false;
    bool drawPoints = true;
    bool ballsCollision = false;
    float timer = 0;
    int chosen_idx = -1;
    int had_collision = -1;

    while (!WindowShouldClose())
    {
        BeginDrawing();
	
            ClearBackground(RAYWHITE);
	    DrawFPS(0, 0);
	    timer += GetFrameTime();

	    if (IsKeyPressed(KEY_L)) {
		drawTrace = !drawTrace;
	    }

	    if (IsKeyPressed(KEY_K)) {
		drawPoints = !drawPoints;
	    }

	    if (IsKeyPressed(KEY_J)) {
		ballsCollision = !ballsCollision;
	    }

	    if (drawTrace) {
		DrawText("drawTrace = true", 15, 0, 13, BLACK);
	    }

	    if (drawPoints) {
		DrawText("drawPoints = true", 15, 15, 13, BLACK);
	    }

	    if (ballsCollision) {
		DrawText("ballsCollision = true", 15, 30, 13, BLACK);
	    }

	    if (chosen_idx >= 0) {
		DrawText(TextFormat("vx: %f", balls.items[chosen_idx].velocity.x), WIDTH - 100, 0, 10, BLACK);
		DrawText(TextFormat("vy: %f", balls.items[chosen_idx].velocity.y), WIDTH - 100, 10, 10, BLACK);
	    }

	    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Ball ball = { .coords = GetMousePosition(), .velocity = { .x = randomValue(100), .y = randomValue(100) }, .color = BLACK };
		da_append(&balls, ball);
	    }

	    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
		da_append(&poly, GetMousePosition());
	    }
	
	    for (size_t i = 0; i < balls.count; i++) {
		// draw balls 
		DrawCircleV(balls.items[i].coords, RADIUS, balls.items[i].color);

		if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
		    if (CheckCollisionCircles(GetMousePosition(), RADIUS, balls.items[i].coords, RADIUS)) {
			chosen_idx = i;
			balls.items[i].color = BLUE;
		    } else {
			if (!ColorIsEqual(balls.items[i].color, BLACK)) {
			    balls.items[i].color = BLACK;
			}
		    }
		}

		// check collision with walls
		if (poly.count >= 2) {
		    for (size_t j = 0; j < poly.count - 1; j++) {
			if (!CheckCollisionCircleLine(balls.items[i].coords, RADIUS, poly.items[j], poly.items[j + 1])) continue; 
			balls.items[i].velocity = calculate_ball_reflect(poly.items[j], poly.items[j + 1], balls.items[i].velocity);
		    }
		}

		// check collision with other balls
		if (balls.count > 1 && ballsCollision) {
		    for (size_t j = 0; j < balls.count; j++) {
			// self-collision case
			if (j == i) continue;
			if (had_collision >= 0 && had_collision == i) continue;
			if (!CheckCollisionCircles(balls.items[i].coords, RADIUS, balls.items[j].coords, RADIUS)) continue;
			// calc reflect for the first ball
			balls.items[i].velocity = calculate_ball_reflect(balls.items[i].coords, balls.items[j].coords, balls.items[i].velocity);
			balls.items[j].velocity = calculate_ball_reflect(balls.items[j].coords, balls.items[i].coords, balls.items[j].velocity);

			had_collision = j;

			if (balls.items[j].velocity.x < 0) {
			    balls.items[j].velocity.x -= balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			} else {
			    balls.items[j].velocity.x += balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			}

			if (balls.items[j].velocity.y < 0) {
			    balls.items[j].velocity.y -= balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			} else {
			    balls.items[j].velocity.y += balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			}

			if (balls.items[i].velocity.x > 0) {
			    balls.items[i].velocity.x -= balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			} else {
			    balls.items[i].velocity.x += balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			}

			if (balls.items[i].velocity.y > 0) {
			    balls.items[i].velocity.y -= balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			} else {
			    balls.items[i].velocity.y += balls.items[i].velocity.x / PERCENT_ENERGY_LEFT_ON_HIT;
			}
		    }
		}

		// move ball
		moveBall(&balls.items[i]);
	    }

	    if (drawPoints) {
		for (size_t i = 0; i < poly.count; i++) {
		    DrawCircleV(poly.items[i], 5, RED);
		}
	    }

	    if (poly.count >= 2) {
		DrawSplineLinear(poly.items, poly.count, 3, RED);
	    }

	    had_collision = -1;
        EndDrawing();
    }

    CloseWindow();

    return 0;
}

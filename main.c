#include <SDL3/SDL.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_keycode.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_video.h>

#include <SDL3_ttf/SDL_ttf.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "font.h"
#include "splash.h"

typedef struct {
	unsigned int p1;
	unsigned int p2;
} POINTS;

typedef enum { PLAYER, OPP } WHO;
typedef enum { EASY, PEACEFULL, HALF, NORMAL, HARD, IMPOSSIABLE } DEFFUCUILTI;

void increase_points(WHO who, POINTS *pc) {
	switch (who) {
	case PLAYER:
		pc->p1++;
		break;
	case OPP:
		pc->p2++;
		break;
	}
}

void change_color(SDL_Color *box) {
	box->r = rand() % (255 - 50 + 1) + 50;
	box->g = rand() % (255 - 50 + 1) + 50;
	box->b = rand() % (255 - 50 + 1) + 50;
}

void increase_speed_r(float *x, float *y) {
	*x = (float)(rand() % 901 + 100);
	*y = (float)(rand() % 801 - 200);

	if (rand() % 2)
		*x = -*x;
	if (rand() % 2)
		*y = -*y;
}

void inc_speed(float *x, float *y) {
	*x += (*x > 0.0f) ? 25.0f : -25.0f;
	*y += (*y > 0.0f) ? 25.0f : -25.0f;
}

void draw_text(SDL_Renderer *ren, TTF_Font *font, const char *text, float x,
			   float y) {
	SDL_Color white = {255, 255, 255, 255};

	SDL_Surface *surface =
		TTF_RenderText_Blended(font, text, strlen(text), white);

	if (surface == NULL)
		return;

	SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);

	if (texture != NULL) {
		float scale = 0.5f;

		SDL_FRect dst = {x, y, surface->w * scale, surface->h * scale};

		SDL_RenderTexture(ren, texture, NULL, &dst);
		SDL_DestroyTexture(texture);
	}

	SDL_DestroySurface(surface);
}

bool check(POINTS *pc, bool *paused) {
	if (pc->p1 >= 11) {
		*paused = true;
		return true;
	}

	if (pc->p2 >= 11) {
		*paused = true;
		return true;
	}

	return false;
}

const char *get_difficulty_name(DEFFUCUILTI difficulty) {
	switch (difficulty) {
	case EASY:
		return "EASY";
	case PEACEFULL:
		return "PEACEFUL";
	case HALF:
		return "HALF";
	case NORMAL:
		return "NORMAL";
	case HARD:
		return "HARD";
	case IMPOSSIABLE:
		return "IMPOSSIBLE";
	default:
		return "UNKNOWN";
	}
}

float get_opp_speed(DEFFUCUILTI d) {
	switch (d) {
	case EASY:
		return 120.0f;
	case PEACEFULL:
		return 180.0f;
	case HALF:
		return 240.0f;
	case NORMAL:
		return 320.0f;
	case HARD:
		return 450.0f;
	case IMPOSSIABLE:
		return 600.0f;
	}
	return 0.0f;
}
int main(void) {

	if (!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Unable To Start SDL3\n");
		return 1;
	}
	if (!TTF_Init()) {
		fprintf(stderr, "TTF_Init failed: %s\n", SDL_GetError());
		return 1;
	}

	SDL_IOStream *font_io = SDL_IOFromConstMem(tiny_ttf, sizeof(tiny_ttf));

	if (font_io == NULL) {
		fprintf(stderr, "Font IO failed: %s\n", SDL_GetError());
		return 1;
	}

	TTF_Font *font = TTF_OpenFontIO(font_io, true, 45.0f);

	if (font == NULL) {
		fprintf(stderr, "Font loading failed: %s\n", SDL_GetError());
		return 1;
	}
	srand((unsigned int)time(NULL));

	SDL_Window *win = SDL_CreateWindow("Gaame", 640, 480, SDL_WINDOW_RESIZABLE);
	if (win == NULL) {
		fprintf(stderr, "Window not created %s", SDL_GetError());
		SDL_Quit();
		return 1;
	}
	SDL_Renderer *ren = SDL_CreateRenderer(win, NULL);
	if (ren == NULL) {
		fprintf(stderr, "Renderer not created %s", SDL_GetError());
		SDL_DestroyWindow(win);
		SDL_Quit();
		return 1;
	}

	bool running = true;
	SDL_Event event;

	bool paused = true;
	bool started = false;
	bool colided = false;

	int which_splash = 0;

	float mouse_x = 0.0f;
	float mouse_y = 0.0f;

	uint64_t frame = 0;

	POINTS pc = {0};

	char score[32];

	int screen_w, screen_h;
	SDL_GetWindowSize(win, &screen_w, &screen_h);

	SDL_FRect rec = {5.0f, 50.0f, 25.0f, 100.0f};
	SDL_FRect box = {-0.0f, -0.f, 20.0f, 20.0f};
	SDL_FRect opp = {screen_w - 35, 50.0f, 25.0f, 100.0f};

	box.x = (float)(rand() % (screen_w - 50 - (int)box.w + 1));
	box.y = (float)(rand() % (screen_h - (int)box.h + 1));
	float box_vx = rand() % 500;
	float box_vy = rand() % 400;
	SDL_Color box_color = {200, 200, 200, 255};
	bool change_box_color = true;

	bool game_over;
	bool fullscreen = false;

	DEFFUCUILTI defi = rand() % 6;

	float opp_speed;
	opp_speed = get_opp_speed(defi);

	Uint64 last_time = SDL_GetPerformanceCounter();

	while (running) {

		Uint64 current_time = SDL_GetPerformanceCounter();

		float dt = (float)(current_time - last_time) /
				   (float)SDL_GetPerformanceFrequency();

		last_time = current_time;
		if (dt > 0.05f)
			dt = 0.05f;

		// Events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT)
				running = false;

			if (event.type == SDL_EVENT_KEY_DOWN) {
				switch (event.key.key) {
				case SDLK_ESCAPE:
					running = false;
					break;

				case SDLK_W:
				case SDLK_UP:
					rec.y -= 50;
					break;

				case SDLK_S:
				case SDLK_DOWN:
					rec.y += 50;
					break;

				case SDLK_SPACE:
				case SDLK_Q:
					paused = !paused;
					started = true;
					break;

				case SDLK_E:
					increase_speed_r(&box_vx, &box_vy);
					break;

				case SDLK_C:
					if (event.key.mod & SDL_KMOD_SHIFT) {
						if (!event.key.repeat)
							change_box_color = !change_box_color;
					} else {
						if (!event.key.repeat)
							change_color(&box_color);
					}
					break;

				case SDLK_D:
					defi = (DEFFUCUILTI)(rand() % 6);
					opp_speed = get_opp_speed(defi);
					break;

				case SDLK_F:
					fullscreen = !fullscreen;
					SDL_SetWindowFullscreen(win, fullscreen);
					break;

				case SDLK_R:
					game_over = false;
					pc.p1 = 0;
					pc.p2 = 0;
					break;
				}
			}

			if (event.type == SDL_EVENT_MOUSE_MOTION) {
				mouse_x = event.motion.x;
				mouse_y = event.motion.y;
			}

			if (event.type == SDL_EVENT_WINDOW_RESIZED) {
				SDL_GetWindowSize(win, &screen_w, &screen_h);
			}
		}

		// Update
		if (rec.x < 0.0f)
			rec.x = 0.0f;

		if (rec.y < 50.0f)
			rec.y = 50.0f;

		if (rec.x + rec.w > screen_w)
			rec.x = screen_w - rec.w;

		if (rec.y + rec.h > screen_h)
			rec.y = screen_h - rec.h;

		opp.x = screen_w - opp.w - 5.0f;

		if (!paused) {
			box.x += box_vx * dt;
			box.y += box_vy * dt;

			bool collision = box.x < rec.x + rec.w && box.x + box.w > rec.x &&
							 box.y < rec.y + rec.h && box.y + box.h > rec.y;

			if (collision && box_vx < 0.0f) {
				box.x = rec.x + rec.w;
				box_vx = -box_vx;

				inc_speed(&box_vx, &box_vy);
				if (change_box_color)
					change_color(&box_color);
				colided = true;
			}

			if (box.x + box.w < 0.0f) {
				increase_points(OPP, &pc);
				box.x = screen_w / 2.0f;
				box.y = screen_h / 2.0f;
			} else if (box.x > screen_w) {
				increase_points(PLAYER, &pc);
				box.x = screen_w / 2.0f;
				box.y = screen_h / 2.0f;
			}
			if (box.y <= 50.0f) {
				box.y = 50.0f;
				box_vy = -box_vy;
			} else if (box.y + box.h >= screen_h) {
				box.y = screen_h - box.h;
				box_vy = -box_vy;
			}

			float opp_center = opp.y + opp.h / 2.0f;
			float box_center = box.y + box.h / 2.0f;

			if (opp_center < box_center) {
				opp.y += opp_speed * dt;
			} else if (opp_center > box_center) {
				opp.y -= opp_speed * dt;
			}
			if (opp.y < 50.0f)
				opp.y = 50.0f;

			if (opp.y + opp.h > screen_h)
				opp.y = screen_h - opp.h;
			bool opp_collision = box.x < opp.x + opp.w &&
								 box.x + box.w > opp.x &&
								 box.y < opp.y + opp.h && box.y + box.h > opp.y;

			if (opp_collision && box_vx > 0.0f) {
				box.x = opp.x - box.w;
				box_vx = -box_vx;

				if (change_box_color)
					change_color(&box_color);

				inc_speed(&box_vx, &box_vy);
				colided = true;
			}
		}

		game_over = check(&pc, &paused);

		if (colided) {
			which_splash = rand() % SPLASH_COUNT;
		}

		// Draw
		SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
		SDL_RenderClear(ren);

		SDL_SetRenderDrawColor(ren, 150, 150, 150, 255);
		SDL_RenderFillRect(ren, &rec);
		SDL_RenderFillRect(ren, &opp);

		SDL_SetRenderDrawColor(ren, box_color.r, box_color.g, box_color.b,
							   box_color.a);
		SDL_RenderFillRect(ren, &box);

		SDL_SetRenderDrawColor(ren, 255, 255, 255, 255);
		SDL_RenderLine(ren, 0, 50, screen_w, 50);

		snprintf(score, sizeof(score), "%u : %u", pc.p1, pc.p2);
		draw_text(ren, font, score, 30 - 15.0f, 15.0f);
		draw_text(ren, font, get_difficulty_name(defi), screen_w - 180, 15.0f);
		if (!started) {
			const char *msg = "First To 11 Wins";

			int text_w, text_h;

			if (!TTF_GetStringSize(font, msg, strlen(msg), &text_w, &text_h)) {
				fprintf(stderr, "Text measurement failed: %s\n",
						SDL_GetError());
			}

			text_w *= 0.5f;
			text_h *= 0.5f;

			draw_text(ren, font, msg, ((float)screen_w - text_w) / 2.0f,
					  ((float)screen_h - text_h) / 2.0f);
		}
		if (game_over) {
			const char *msg = (pc.p1 >= 11) ? "You Won!" : "You Lost!";

			int text_w, text_h;

			if (!TTF_GetStringSize(font, msg, strlen(msg), &text_w, &text_h)) {
				fprintf(stderr, "Text measurement failed: %s\n",
						SDL_GetError());
			}

			text_w *= 0.5f;
			text_h *= 0.5f;

			draw_text(ren, font, msg, ((float)screen_w - text_w) / 2.0f,
					  ((float)screen_h - text_h) / 2.0f);
		}

		int splash_w, splash_h;

		if (!TTF_GetStringSize(font, splash[which_splash],
							   strlen(splash[which_splash]), &splash_w,
							   &splash_h)) {
			fprintf(stderr, "Text measurement failed: %s\n", SDL_GetError());
		}

		splash_w *= 0.5f;
		splash_h *= 0.5f;

		draw_text(ren, font, splash[which_splash],
				  (float)screen_w / 2 - splash_w, 15.0f);

		SDL_RenderPresent(ren);

		frame++;
		colided = false;
	}
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(win);
	TTF_CloseFont(font);
	TTF_Quit();
	SDL_Quit();
}

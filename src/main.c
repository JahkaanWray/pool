#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <SDL2/SDL.h>

typedef struct
{
    double x;
    double y;
    double z;
} Vector3;

typedef struct
{
    Vector3 initial_position;
    Vector3 initial_velocity;
    Vector3 initial_angular_velocity;
    bool rolling;
} PathSegment;

double Vector3_dot(Vector3 v, Vector3 w)
{
    return v.x * w.x + v.y * w.y + v.z * w.z;
}

Vector3 Vector3_scalar_multiply(Vector3 v, double s)
{
    Vector3 result = {v.x * s, v.y * s, v.z * s};
    return result;
}

Vector3 Vector3_add(Vector3 v, Vector3 w)
{
    Vector3 result = {v.x + w.x, v.y + w.y, v.z + w.z};
    return result;
}

Vector3 Vector3_subtract(Vector3 v, Vector3 w)
{
    Vector3 result = {v.x - w.x, v.y - w.y, v.z - w.z};
    return result;
}

Vector3 Vector3_cross(Vector3 v, Vector3 w)
{
    Vector3 result = {
        v.y * w.z - v.z * w.y,
        v.z * w.x - v.x * w.z,
        v.x * w.y - v.y * w.x};
    return result;
}

double Vector3_mag(Vector3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vector3 Vector3_normalize(Vector3 v)
{
    double mag = Vector3_mag(v);
    return Vector3_scalar_multiply(v, 1.0 / mag);
}

Vector3 render_path(SDL_Renderer *renderer, Vector3 v, Vector3 w)
{
    double R = 1.0;
    double mu_slide = 0.5;
    double mu_roll = 0.3;
    double g = 9.8;
    Vector3 k = {0, 0, 1};
    Vector3 w_cross_R = Vector3_scalar_multiply(Vector3_cross(w, k), R);
    Vector3 contact_point_v = Vector3_subtract(v, w_cross_R);
    Vector3 contact_point_v_normalized = Vector3_normalize(contact_point_v);
    Vector3 v_roll = Vector3_add(Vector3_scalar_multiply(v, 5.0 / 7.0), Vector3_scalar_multiply(w_cross_R, 2.0 / 7.0));
    Vector3 v_roll_normalized = Vector3_normalize(v_roll);
    double slide_time = 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
    double roll_time = Vector3_mag(v_roll) / (mu_roll * g);
    for (int i = 0; i < 100; i++)
    {
        double t1 = i * slide_time / 100;
        double t2 = (i + 1) * slide_time / 100;

        Vector3 p1 = Vector3_subtract(Vector3_scalar_multiply(v, t1), Vector3_scalar_multiply(contact_point_v_normalized, 0.5 * mu_slide * g * t1 * t1));
        Vector3 p2 = Vector3_subtract(Vector3_scalar_multiply(v, t2), Vector3_scalar_multiply(contact_point_v_normalized, 0.5 * mu_slide * g * t2 * t2));
        SDL_RenderDrawLine(renderer, p1.x, p1.y, p2.x, p2.y);
    }
    Vector3 r_slide = Vector3_subtract(Vector3_scalar_multiply(v, slide_time), Vector3_scalar_multiply(contact_point_v_normalized, 0.5 * mu_slide * g * slide_time * slide_time));
    Vector3 r_roll = Vector3_subtract(Vector3_scalar_multiply(v_roll, roll_time), Vector3_scalar_multiply(v_roll_normalized, 0.5 * mu_roll * g * roll_time * roll_time));

    Vector3 r = Vector3_add(r_slide, r_roll);

    SDL_RenderDrawLine(renderer, r_slide.x, r_slide.y, r.x, r.y);

    printf("Contact point velocity = (%f, %f, %f)\n", contact_point_v.x, contact_point_v.y, contact_point_v.z);
    printf("Time spent sliding = %f\n", slide_time);
    printf("Time spent rolling = %f\n", roll_time);
    printf("Total displacement = (%f, %f, %f)\n", r.x, r.y, r.z);
    return r;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1640, 900, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Vector3 v = {1, 0, 0};
    Vector3 w = {0, 1, 0};
    int v_mag = 1;
    int w_mag = 1;

    bool quit = 0;
    SDL_Event event;
    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if (event.type == SDL_MOUSEMOTION)
            {
                int mx, my;
                Uint32 mouseState = SDL_GetMouseState(&mx, &my);
                if (mx > 1450 && mx < 1530 && my > 10 && my < 890)
                {
                    v_mag = 890 - my;
                }
                if (mx > 1550 && mx < 1630 && my > 10 && my < 890)
                {
                    w_mag = 890 - my;
                }
                if (mx > 0 && mx < 100 && my > 700 && my < 800)
                {
                    v = Vector3_normalize((Vector3){mx - 50, my - 750, 0});
                }
                if (mx > 0 && mx < 100 && my > 800 && my < 900)
                {
                    w = Vector3_normalize((Vector3){mx - 50, my - 850, 0});
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 700, 100, 100});
        SDL_RenderFillRect(renderer, &(SDL_Rect){0, 800, 100, 100});
        SDL_RenderFillRect(renderer, &(SDL_Rect){1540, 0, 100, 900});
        SDL_RenderFillRect(renderer, &(SDL_Rect){1440, 0, 100, 900});
        SDL_SetRenderDrawColor(renderer, 180, 0, 180, 255);
        SDL_RenderFillRect(renderer, &(SDL_Rect){1550, 890 - w_mag, 80, w_mag});
        SDL_RenderFillRect(renderer, &(SDL_Rect){1450, 890 - v_mag, 80, v_mag});
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 50, 750, 50 + 50 * v.x, 750 + 50 * v.y);
        SDL_RenderDrawLine(renderer, 50, 850, 50 + 50 * w.x, 850 + 50 * w.y);

        render_path(renderer, Vector3_scalar_multiply(v, v_mag), Vector3_scalar_multiply(w, w_mag));
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
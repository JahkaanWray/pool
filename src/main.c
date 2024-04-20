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

int calculate_displacement()
{
    double R = 1.0;
    double mu_slide = 0.5;
    double mu_roll = 0.3;
    double g = 9.8;
    Vector3 v = {20.0, 0.0, 0};
    Vector3 w = {10.0, 0.0, 0};
    Vector3 k = {0, 0, 1};
    Vector3 w_cross_R = Vector3_scalar_multiply(Vector3_cross(w, k), R);
    Vector3 contact_point_v = Vector3_subtract(v, w_cross_R);
    Vector3 contact_point_v_normalized = Vector3_normalize(contact_point_v);
    Vector3 v_roll = Vector3_add(Vector3_scalar_multiply(v, 5.0 / 7.0), Vector3_scalar_multiply(w_cross_R, 2.0 / 7.0));
    Vector3 v_roll_normalized = Vector3_normalize(v_roll);
    double slide_time = 2 * Vector3_mag(contact_point_v) / (7 * mu_slide * g);
    double roll_time = Vector3_mag(v_roll) / (mu_roll * g);
    Vector3 r_slide = Vector3_subtract(Vector3_scalar_multiply(v, slide_time), Vector3_scalar_multiply(contact_point_v_normalized, 0.5 * mu_slide * g * slide_time * slide_time));
    Vector3 r_roll = Vector3_subtract(Vector3_scalar_multiply(v_roll, roll_time), Vector3_scalar_multiply(v_roll_normalized, 0.5 * mu_roll * g * roll_time * roll_time));

    Vector3 r = Vector3_add(r_slide, r_roll);

    printf("Contact point velocity = (%f, %f, %f)\n", contact_point_v.x, contact_point_v.y, contact_point_v.z);
    printf("Time spent sliding = %f\n", slide_time);
    printf("Time spent rolling = %f\n", roll_time);
    printf("Total displacement = (%f, %f, %f)\n", r.x, r.y, r.z);
    return 0;
}

int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

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
        }
        SDL_SetRenderDrawColor(renderer, 0, 180, 0, 255);
        SDL_RenderClear(renderer);
        SDL_RenderPresent(renderer);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
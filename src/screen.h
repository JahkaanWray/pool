#ifndef SCREEN_H
#define SCREEN_H
struct Screen;

typedef struct Screen
{
    struct Screen *(*update)(struct Screen *screen);
    void (*render)(struct Screen *screen);
} Screen;

#endif // SCREEN_H
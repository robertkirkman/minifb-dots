
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <MiniFB.h>

#define WIDTH 800
#define HEIGHT 600
#define WHITE MFB_ARGB(0xFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF)
#define BLACK MFB_ARGB(0xFF, 0x0, 0x0, 0x0)

// struct to store cartesion point data
typedef struct _Point
{
    size_t x, y;
} Point;

// convert polar coordinates to cartesian coordinates within the bounds of our framebuffer
Point pol2cart(double rho, double phi)
{
    Point p;
    double x = rho * cos(phi) + WIDTH / 2,
           y = rho * sin(phi) + HEIGHT / 2;
    p.x = x > 0 && x < WIDTH ? (size_t)x : 0;
    p.y = y > 0 && y < HEIGHT ? (size_t)y : 0;
    return p;
}

// print a gradient square black dot into the given framebuffer
void dot(uint32_t *framebuffer, Point p, unsigned radius)
{
    if (p.x < radius || p.y < radius || p.x >= WIDTH || p.y >= HEIGHT)
        return;
    for (size_t i = p.x - radius; i < p.x + radius && i < WIDTH * HEIGHT; i++)
    {
        for (size_t j = p.y - radius; j < p.y + radius && i < WIDTH * HEIGHT; j++)
        {
            uint32_t color = (abs((int)i - (int)p.x) + abs((int)j - (int)p.y)) * 20;
            framebuffer[i + WIDTH * j] = MFB_ARGB(0xFF, color, color, color);
        }
    }
}

int main(void)
{
    struct mfb_window *window = mfb_open_ex("minifbproject", WIDTH, HEIGHT, WF_RESIZABLE);
    if (!window)
        return 1;

    int state, count = 0, decreasing = 0, size = 5;
    double rho = 100, phi = 0;

    uint32_t *framebuffer = malloc(WIDTH * HEIGHT * sizeof(uint32_t *));

    // initialize white
    for (size_t i = 0; i < WIDTH * HEIGHT; i++)
    {
        framebuffer[i] = WHITE;
    }

    do
    {
        /*
         * CHANGE THE FRAMEBUFFER CONTENTS HERE up to every frame
         */
        Point p = pol2cart(rho, phi), mouse;
        mouse.x = (size_t)mfb_get_mouse_x(window);
        mouse.y = (size_t)mfb_get_mouse_y(window);
        int lmb = mfb_get_mouse_button_buffer(window)[1];
        int rmb = mfb_get_mouse_button_buffer(window)[2];
        size += lmb - rmb;
        if (size < 1)
            size = 1;

        for (size_t i = 0; i < WIDTH * HEIGHT; i++)
        {
            if (i == p.x + WIDTH * p.y)
                dot(framebuffer, p, 5);
            if (i == mouse.x + WIDTH * mouse.y)
                dot(framebuffer, mouse, size);
        }

        // every frame count increases up to WIDTH, then decreases
        if (decreasing)
            count--;
        else
            count++;

        // increase angle of simulated compass at intervals.
        // this is effectively a clock based on framerate.
        // that is how old-school video games were written,
        // but for large projects today, a framerate-independent time library
        // should be used instead. this is used for convenience when it
        // *doesn't matter exactly how fast the clock runs*
        if (count == 5)
        {
            phi = phi + 0.1;
            decreasing = 1;
        }
        if (count == 0)
            decreasing = 0;

        /*
         * END changing the framebuffer contents here
         */

        state = mfb_update_ex(window, framebuffer, WIDTH, HEIGHT);

        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));

    free(framebuffer);

    return 0;
}
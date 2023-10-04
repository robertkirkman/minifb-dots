#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <MiniFB.h>

// Define constants for the framebuffer size and colors
#define WIDTH 800
#define HEIGHT 600
#define WHITE MFB_ARGB(0xFF, 0x7FFFFFFF, 0x7FFFFFFF, 0x7FFFFFFF)
#define BLACK MFB_ARGB(0xFF, 0x0, 0x0, 0x0)

// Structure to store Cartesian point data
typedef struct _Point
{
    size_t x, y;
} Point;

// Convert polar coordinates to Cartesian coordinates within the bounds of our framebuffer
Point polarToCartesian(double rho, double phi)
{
    Point p;
    double x = rho * cos(phi) + WIDTH / 2;
    double y = rho * sin(phi) + HEIGHT / 2;

    // Ensure the resulting point is within the framebuffer bounds
    p.x = (x >= 0 && x < WIDTH) ? (size_t)x : 0;
    p.y = (y >= 0 && y < HEIGHT) ? (size_t)y : 0;

    return p;
}

// Draw a gradient square dot into the given framebuffer
void drawDot(uint32_t *framebuffer, Point p, unsigned radius)
{
    if (p.x < radius || p.y < radius || p.x >= WIDTH || p.y >= HEIGHT)
        return;

    for (size_t i = p.x - radius; i < p.x + radius && i < WIDTH; i++)
    {
        for (size_t j = p.y - radius; j < p.y + radius && j < HEIGHT; j++)
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

    int state, count = 0, decreasing = 0, size = 5, delay = 5;
    double rho = 100, phi = 0;

    uint32_t *framebuffer = malloc(WIDTH * HEIGHT * sizeof(uint32_t));

    // Initialize the framebuffer with white
    for (size_t i = 0; i < WIDTH * HEIGHT; i++)
    {
        framebuffer[i] = WHITE;
    }

    do
    {
        // Update polar coordinates based on mouse position and button clicks
        Point cartesianFromPolar = polarToCartesian(rho, phi);
        Point mouse;
        mouse.x = (size_t)mfb_get_mouse_x(window);
        mouse.y = (size_t)mfb_get_mouse_y(window);
        int leftMouseButton = mfb_get_mouse_button_buffer(window)[1];
        int rightMouseButton = mfb_get_mouse_button_buffer(window)[2];
        size += leftMouseButton - rightMouseButton;

        if (size < 1)
            size = 1;

        // Draw dots at the polar and mouse positions
        drawDot(framebuffer, cartesianFromPolar, 5);
        drawDot(framebuffer, mouse, size);

        // Update the frame count, increasing it and decreasing it between 0 and delay (5)
        // this means 60 / (5 + 5) times per second, or 6 times per second
        if (decreasing)
            count--;
        else
            count++;

        // Increase the angle of the dot for plotting a circle
        // at intervals relative to the framerate
        if (count == delay)
        {
            phi += 0.1;
            decreasing = 1;
        }
        if (count == 0)
            decreasing = 0;

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
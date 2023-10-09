#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <MiniFB.h>

// Define constants for the framebuffer size and colors

// 1111 1111 0000 0000 0000 0000 0000 0000
// 0xFF 11111111 255
//
#define WIDTH 800
#define HEIGHT 600
#define WHITE MFB_ARGB(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK MFB_ARGB(0xFF, 0x00, 0x00, 0x00)

// Structure to store Cartesian point data
typedef struct Dot
{
    int x, y, radius;
} Dot;

typedef struct DotList DotList;
struct DotList
{
    DotList *next;
    Dot p;
    clock_t timestamp;
};

// Draw a circular dot into the given framebuffer
void drawDot(uint32_t *fb, Dot p)
{
    for (int i = p.x - p.radius; i < p.x + p.radius && i < WIDTH; i++)
    {
        for (int j = p.y - p.radius; j < p.y + p.radius && j < HEIGHT; j++)
        {
            double dist = sqrt(pow((double)i - (double)p.x, 2) + pow((double)j - (double)p.y, 2));
            if (dist <= p.radius && i >= 0 && j >= 0)
            {
                uint32_t color = (uint32_t)dist;
                fb[i + WIDTH * j] = MFB_ARGB(0xFF, 0x00, color, 0x00);
            }
        }
    }
}

void addDot(struct mfb_window *window, uint32_t *fb, DotList **pl, int radius, clock_t timestamp)
{
    DotList *last = NULL;
    while (*pl != NULL)
    {
        last = *pl;
        *pl = (*pl)->next;
    }

    *pl = malloc(sizeof(struct DotList));
    (*pl)->p.x = mfb_get_mouse_x(window);
    (*pl)->p.y = mfb_get_mouse_y(window);
    (*pl)->p.radius = radius;
    (*pl)->timestamp = timestamp;
    (*pl)->next = NULL;
    if (last != NULL)
        last->next = *pl;
    drawDot(fb, (*pl)->p);
}

DotList *captureDots(uint32_t *fb)
{
    int state, size = 5;
    DotList *head = NULL, *pl = NULL;
    clock_t start = clock();
    struct mfb_window *window = mfb_open_ex("recording dots...", WIDTH, HEIGHT, WF_RESIZABLE);
    do
    {
        int leftMouseButton = mfb_get_mouse_button_buffer(window)[MOUSE_BTN_1];
        int rightMouseButton = mfb_get_mouse_button_buffer(window)[MOUSE_BTN_2];
        int spaceBar = mfb_get_key_buffer(window)[KB_KEY_SPACE];

        size += leftMouseButton - rightMouseButton;

        if (size < 1)
            size = 1;

        if (spaceBar)
            addDot(window, fb, &pl, size, clock() - start);
        if (head == NULL)
            head = pl;

        state = mfb_update_ex(window, fb, WIDTH, HEIGHT);

        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));
    return head;
}

void playbackDots(uint32_t *fb, DotList *pl)
{
    int state, size;
    clock_t start = clock();
    struct mfb_window *window = mfb_open_ex("displaying recorded dots...", WIDTH, HEIGHT, WF_RESIZABLE);
    do
    {
        if (pl != NULL && clock() >= start + pl->timestamp)
        {
            drawDot(fb, pl->p);
            pl = pl->next;
        }

        state = mfb_update_ex(window, fb, WIDTH, HEIGHT);

        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));
}

void freeDots(DotList *pl)
{
    DotList *tmp;
    while (pl != NULL)
    {
        tmp = pl->next;
        free(pl);
        pl = tmp;
    }
}

void clearFramebuffer(uint32_t *fb)
{
    // Initialize the framebuffer with white
    for (size_t i = 0; i < WIDTH * HEIGHT; i++)
    {
        fb[i] = WHITE;
    }
}

int main(void)
{
    uint32_t *framebuffer = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    if (framebuffer == NULL)
        return 2;

    clearFramebuffer(framebuffer);
    DotList *dotlist = captureDots(framebuffer);
    clearFramebuffer(framebuffer);
    playbackDots(framebuffer, dotlist);

    freeDots(dotlist);
    free(framebuffer);

    return 0;
}
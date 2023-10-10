#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>
#include <MiniFB.h>

// Define constants for the framebuffer size and colors
#define WIDTH 800
#define HEIGHT 600
#define WHITE MFB_ARGB(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK MFB_ARGB(0xFF, 0x00, 0x00, 0x00)

// Structure to store Cartesian point data and radius of dot
typedef struct Dot
{
    int x, y, radius;
} Dot;

// this syntax for defining a struct is used to create linked-lists.
// DotList is an example of the: single-linked-list data structure!
// a linked-list has multiple "container" objects that each contain at least
// one pointer to the "next" container object in the list. these container
// objects can then each contain separate instances of data objects or other 
// members.
typedef struct DotList DotList;
struct DotList
{
    DotList *next;
    Dot p;
    clock_t timestamp;
};

// If the condition is not met then exit the program with error message.
void check(_Bool condition, const char *func, int line)
{
    if (condition)
        return;
    perror(func);
    fprintf(stderr, "%s failed in file %s at line # %d\n", func, __FILE__, line - 1);
    exit(EXIT_FAILURE);
}

// Draw a circular dot into the given framebuffer
void drawDot(uint32_t *fb, Dot p)
{
    // use of signed integers allows natural calculations for partially-visible dots
    for (int i = p.x - p.radius; i < p.x + p.radius && i < WIDTH; i++)
    {
        for (int j = p.y - p.radius; j < p.y + p.radius && j < HEIGHT; j++)
        {
            // this distance formula defines the circle
            double dist = sqrt(pow((double)i - (double)p.x, 2) + pow((double)j - (double)p.y, 2));
            if (dist <= p.radius && i >= 0 && j >= 0)
            {
                uint32_t color = (uint32_t)dist;
                fb[i + WIDTH * j] = MFB_ARGB(0xFF, 0x00, color, 0x00);
            }
        }
    }
}

// capture, append a dot to the DotList and render the dot on the screen
void addDot(struct mfb_window *window, uint32_t *fb, DotList **pl, int radius, clock_t timestamp)
{
    DotList *last = NULL;

    // if the DotList is not advanced to the last node, advance it there
    while (*pl != NULL)
    {
        last = *pl;
        *pl = (*pl)->next;
    }

    // allocate new node and initialize members
    *pl = malloc(sizeof(struct DotList));
    check(*pl != NULL, "malloc()", __LINE__);
    (*pl)->p.x = mfb_get_mouse_x(window);
    (*pl)->p.y = mfb_get_mouse_y(window);
    (*pl)->p.radius = radius;
    (*pl)->timestamp = timestamp;
    (*pl)->next = NULL;
    if (last != NULL)
        last->next = *pl;

    // render the new dot
    drawDot(fb, (*pl)->p);
}

// loop indefinitely and capture all dots created during the loop, including their "timestamp",
// a delay counter representing the time elapsed in between the window opening and the dot being placed
DotList *captureDots(uint32_t *fb)
{
    int state, size = 5;
    DotList *head = NULL, *pl = NULL;
    // record initial time just before opening the window - the delay timing of the recorded dots starts here
    clock_t start = clock();
    struct mfb_window *window = mfb_open_ex("recording dots...", WIDTH, HEIGHT, WF_RESIZABLE);
    check(window != NULL, "mfb_open_ex()", __LINE__);
    do
    {
        int leftMouseButton = mfb_get_mouse_button_buffer(window)[MOUSE_BTN_1];
        int rightMouseButton = mfb_get_mouse_button_buffer(window)[MOUSE_BTN_2];
        int spaceBar = mfb_get_key_buffer(window)[KB_KEY_SPACE];

        size += leftMouseButton - rightMouseButton;

        if (size < 1)
            size = 1;

        // every frame during which spacebar is pressed, add a new dot of the current size and position to the DotList
        // and render it on the screen
        if (spaceBar)
            addDot(window, fb, &pl, size, clock() - start);
        
        // preserve the first node of the DotList once it's been collected
        if (head == NULL && pl != NULL)
            head = pl;

        state = mfb_update_ex(window, fb, WIDTH, HEIGHT);

        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));

    // return pointer to the preserved first node, so playback can start at the beginning
    return head;
}

void playbackDots(uint32_t *fb, DotList *pl)
{
    int state;
    // record initial time just before opening the window - the dot delay playback timing starts here
    clock_t start = clock();
    struct mfb_window *window = mfb_open_ex("displaying recorded dots...", WIDTH, HEIGHT, WF_RESIZABLE);
    check(window != NULL, "mfb_open_ex()", __LINE__);
    do
    {
        // draw all dots in the correct order and with the correct delays between them by using
        // clock() and advancing the DotList pointer pl through the DotList
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

// free all memory used by dots by advancing through the DotList node by node
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

// Initialize the framebuffer with white
void clearFramebuffer(uint32_t *fb)
{
    for (size_t i = 0; i < WIDTH * HEIGHT; i++)
        fb[i] = WHITE;
}

int main(void)
{
    // allocate framebuffer
    uint32_t *framebuffer = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    check(framebuffer != NULL, "malloc()", __LINE__);

    // initialize framebuffer with white, then allow drawing
    clearFramebuffer(framebuffer);
    DotList *dotlist = captureDots(framebuffer);

    // reinitialize framebuffer with white, then play back the drawing
    clearFramebuffer(framebuffer);
    playbackDots(framebuffer, dotlist);

    // free memory
    freeDots(dotlist);
    free(framebuffer);

    return EXIT_SUCCESS;
}
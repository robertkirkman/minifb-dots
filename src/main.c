#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <MiniFB.h>

// Define constants for the framebuffer size and colors
#define WIDTH 800
#define HEIGHT 600
#define WHITE MFB_ARGB(0xFF, 0xFF, 0xFF, 0xFF)
#define BLACK MFB_ARGB(0xFF, 0x00, 0x00, 0x00)

// Structure to store Cartesian point data and radius of dot
typedef struct
{
    int x, y, radius;
} Dot;

typedef struct
{
    Dot data;
    int velx, vely;
} Player;

// Function prototypes
void checkCondition(_Bool condition, const char *func, int line);
void drawDot(uint32_t *fb, Dot p);
void checkCollide(Player *p);
void clearFramebuffer(uint32_t *fb);

#define integer_str_len 11
#define longfloat_str_len 26
#define combined_str_len (integer_str_len + longfloat_str_len - 1)

unsigned digitCount(int integer, double longfloat) {
    unsigned index = 0;
    char integer_str[combined_str_len] =  { 0 },
         longfloat_str[longfloat_str_len] = { 0 };
    if (integer != 0) {
        snprintf(integer_str, integer_str_len, "%d", integer);
    }
    if (longfloat != 0) {
        snprintf(longfloat_str, longfloat_str_len, "%lf", longfloat);
    }
    index = strlen(integer_str);
    for (unsigned i = 0; i < longfloat_str_len; i++) {
        if (isdigit(longfloat_str[i])) {
            index++;
        }
    }
    return index;
}

int main(void)
{
    int a = 1234;
    double b = 362837.12899800;
    printf("%u\n", digitCount(a, 0));
    printf("%u\n", digitCount(0, b));
    printf("%u\n", digitCount(a, b));




    // Allocate framebuffer
    uint32_t *framebuffer = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    checkCondition(framebuffer != NULL, "malloc()", __LINE__);

    // Initialize framebuffer with white and capture dots
    clearFramebuffer(framebuffer);

    Player p;
    p.data.x = WIDTH / 2;
    p.data.y = HEIGHT / 2;
    p.data.radius = 20;
    p.velx = 0;
    p.vely = 0;

    int state;
    struct mfb_window *window = mfb_open_ex("dot", WIDTH, HEIGHT, WF_RESIZABLE);
    checkCondition(window != NULL, "mfb_open_ex()", __LINE__);
    do
    {
        int leftArrow = mfb_get_key_buffer(window)[KB_KEY_LEFT];
        int rightArrow = mfb_get_key_buffer(window)[KB_KEY_RIGHT];
        int spaceBar = mfb_get_key_buffer(window)[KB_KEY_SPACE];
        
        // left and right movement
        if (leftArrow)
        {
            p.velx = -10;
        }
        else if (rightArrow)
        {
            p.velx = 10;
        }
        else
        {
            p.velx = 0;
        }
        
        // jump movement
        if (spaceBar && p.data.y > HEIGHT - p.data.radius - 2)
        {
            p.vely = -40;
        }
        
        // gravity movement
        if (p.data.y < HEIGHT - p.data.radius - 2)
        {
            p.vely = p.vely + 2;
        }
        else if (!spaceBar)
        {
            // bounce with friction
            p.vely = -p.vely + 2;
        }
        

        checkCollide(&p);

        p.data.x += p.velx;
        p.data.y += p.vely;

        clearFramebuffer(framebuffer);
        drawDot(framebuffer, p.data);

        state = mfb_update_ex(window, framebuffer, WIDTH, HEIGHT);
        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));

    // Free memory
    free(framebuffer);

    return EXIT_SUCCESS;
}

// Check a condition and exit the program with an error message if it's not met
void checkCondition(_Bool condition, const char *func, int line)
{
    if (condition)
    {
        return;
    }
    perror(func);
    fprintf(stderr, "%s failed in file %s at line # %d\n", func, __FILE__, line - 1);
    exit(EXIT_FAILURE);
}

// Draw a circular dot into the given framebuffer
void drawDot(uint32_t *fb, Dot p)
{
    const int x = p.x;
    const int y = p.y;
    const int r = p.radius;
    const int rSquared = r * r;

    for (int i = x - r; i <= x + r && i < WIDTH; i++)
    {
        for (int j = y - r; j <= y + r && j < HEIGHT; j++)
        {
            if (i >= 0 && j >= 0)
            {
                const int dx = i - x;
                const int dy = j - y;
                const int distSquared = dx * dx + dy * dy;

                if (distSquared <= rSquared)
                {
                    uint32_t color = distSquared;
                    fb[i + WIDTH * j] = MFB_ARGB(0xFF, 0x00, color, 0x00);
                }
            }
        }
    }
}

// Check for player collision with edge of window
void checkCollide(Player *p)
{
    const int x = p->data.x;
    const int y = p->data.y;
    const int r = p->data.radius;
    const int rSquared = r * r;

    for (int i = x - r; i <= x + r; i++)
    {
        for (int j = y - r; j <= y + r; j++)
        {
            if (i < 0)
            {
                p->data.x = p->data.radius;
                p->velx = abs(p->velx);
            }
            if (j < 0)
            {
                p->data.y = p->data.radius;
                p->vely = abs(p->vely);
            }
            if (i >= WIDTH)
            {
                p->data.x = WIDTH - p->data.radius;
                p->velx = -abs(p->velx);
            }
            if (j >= HEIGHT)
            {
                p->data.y = HEIGHT - p->data.radius;
                p->vely = -abs(p->vely);
            }
        }
    }
}

// Initialize the framebuffer with white
void clearFramebuffer(uint32_t *fb)
{
    const size_t numPixels = WIDTH * HEIGHT;
    for (size_t i = 0; i < numPixels; i++)
    {
        fb[i] = WHITE;
    }
}

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
typedef struct
{
    int x, y, radius;
} Dot;

// Define a struct for a linked list node containing Dot data
typedef struct DotListNode
{
    struct DotListNode *next;
    Dot data;
    clock_t timestamp;
} DotListNode;

// Function prototypes
void checkCondition(_Bool condition, const char *func, int line);
void drawDot(uint32_t *fb, Dot p);
void addDot(struct mfb_window *window, uint32_t *fb, DotListNode **head, int radius, clock_t timestamp);
DotListNode *captureDots(uint32_t *fb);
void playbackDots(uint32_t *fb, DotListNode *head);
void printDots(DotListNode *head);
void freeDots(DotListNode *head);
void clearFramebuffer(uint32_t *fb);

int main(void)
{
    // Allocate framebuffer
    uint32_t *framebuffer = malloc(WIDTH * HEIGHT * sizeof(uint32_t));
    checkCondition(framebuffer != NULL, "malloc()", __LINE__);

    // Initialize framebuffer with white and capture dots
    clearFramebuffer(framebuffer);
    DotListNode *dotList = captureDots(framebuffer);

    // Reinitialize framebuffer with white and play back the drawing
    clearFramebuffer(framebuffer);
    playbackDots(framebuffer, dotList);

    // Print dotList in c-struct literal format
    printDots(dotList);

    // preallocate and preinitialize a DotList for smiley face :)
    const size_t smileyDotCount = 1;
    DotListNode smileyDotList[] = {
#include <dots.inc>
    };
    // link each element together to form the singly-linked-list
    for (size_t i = 0; i < smileyDotCount - 1; i++) 
    {
        smileyDotList[i].next = &smileyDotList[i + 1];
    }

    // Reinitialize framebuffer with white and play back the drawing of smiley face :)
    clearFramebuffer(framebuffer);
    playbackDots(framebuffer, smileyDotList);

    // Free memory
    freeDots(dotList);
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

// Capture, append a dot to the DotList, and render the dot on the screen
void addDot(struct mfb_window *window, uint32_t *fb, DotListNode **head, int radius, clock_t timestamp)
{
    DotListNode *last = NULL;
    while (*head != NULL)
    {
        last = *head;
        *head = (*head)->next;
    }
    *head = malloc(sizeof(DotListNode));
    checkCondition(*head != NULL, "malloc()", __LINE__);
    (*head)->data.x = mfb_get_mouse_x(window);
    (*head)->data.y = mfb_get_mouse_y(window);
    (*head)->data.radius = radius;
    (*head)->timestamp = timestamp;
    (*head)->next = NULL;
    if (last != NULL)
    {
        last->next = *head;
    }
    drawDot(fb, (*head)->data);
}

// Loop indefinitely and capture all dots created during the loop, including their "timestamp"
DotListNode *captureDots(uint32_t *fb)
{
    int state, size = 5;
    DotListNode *head = NULL, *currentNode = NULL;
    clock_t start = clock();
    struct mfb_window *window = mfb_open_ex("recording dots...", WIDTH, HEIGHT, WF_RESIZABLE);
    checkCondition(window != NULL, "mfb_open_ex()", __LINE__);
    do
    {
        int leftMouseButton = mfb_get_mouse_button_buffer(window)[MOUSE_BTN_1];
        int rightMouseButton = mfb_get_mouse_button_buffer(window)[MOUSE_BTN_2];
        int spaceBar = mfb_get_key_buffer(window)[KB_KEY_SPACE];
        size += leftMouseButton - rightMouseButton;
        if (size < 1)
        {
            size = 1;
        }
        if (spaceBar)
        {
            addDot(window, fb, &currentNode, size, clock() - start);
        }
        if (head == NULL && currentNode != NULL)
        {
            head = currentNode;
        }
        state = mfb_update_ex(window, fb, WIDTH, HEIGHT);
        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));
    return head;
}

// Draw all dots in the correct order with the correct delays between them
void playbackDots(uint32_t *fb, DotListNode *currentNode)
{
    int state;
    clock_t start = clock();
    struct mfb_window *window = mfb_open_ex("displaying recorded dots...", WIDTH, HEIGHT, WF_RESIZABLE);
    checkCondition(window != NULL, "mfb_open_ex()", __LINE__);
    do
    {
        if (currentNode != NULL && clock() >= start + currentNode->timestamp)
        {
            drawDot(fb, currentNode->data);
            currentNode = currentNode->next;
        }
        state = mfb_update_ex(window, fb, WIDTH, HEIGHT);
        if (state < 0)
        {
            window = NULL;
            break;
        }
    } while (mfb_wait_sync(window));
}

// print dots in a portable C-struct plaintext format
void printDots(DotListNode *head)
{
    while (head != NULL)
    {
        printf("{.data = {.x = %d,\n"
               "          .y = %d,\n"
               "          .radius = %d},\n"
               " .timestamp = %ld},\n", 
               head->data.x, 
               head->data.y, 
               head->data.radius, 
               head->timestamp);
        head = head->next;
    }
}

// Free all memory used by dots
void freeDots(DotListNode *head)
{
    DotListNode *tmp;
    while (head != NULL)
    {
        tmp = head->next;
        free(head);
        head = tmp;
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

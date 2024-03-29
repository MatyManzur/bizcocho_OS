#ifndef PRINTING_H
#define PRINTING_H

#include <lib.h>

// colores
typedef enum color_t
{
    DEFAULT = -1,
    BLACK = 0,
    BLUE,
    GREEN,
    CYAN,
    RED,
    MAGENTA,
    BROWN,
    L_GRAY,
    D_GRAY,
    L_BLUE,
    L_GREEN,
    L_CYAN,
    L_RED,
    PINK,
    YELLOW,
    WHITE
} color_t;

// struct que se guarda color de fondo y color de caracteres
typedef struct format_t
{
    color_t backgroundColor;
    color_t characterColor;
} format_t;

// struct de un punto en la pantalla
typedef struct point_t
{
    uint8_t row;
    uint8_t column;
} point_t;
// KERNELs
uint8_t printChar(char character, color_t backgroundColor, color_t characterColor);

char *print(const char *string, const struct format_t *format);

uint8_t newLine();

void scrollUp(uint8_t rows);

// User

void clearScreen();

format_t changeColor(color_t backgroundColor, color_t characterColor);

void setBackspaceBase();

#endif

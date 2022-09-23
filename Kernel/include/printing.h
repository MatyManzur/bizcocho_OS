#ifndef PRINTING_H
#define PRINTING_H

#include <stdint.h>
#include <scheduler.h>

#define NULL 0
#define MAX_SCREEN_COUNT 16

//colores
typedef enum color_t
{
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

//struct que se guarda color de fondo y color de caracteres
typedef struct format_t
{
    color_t backgroundColor;
    color_t characterColor;
} format_t;

//struct de un punto en la pantalla
typedef struct point_t
{
    uint8_t row;
    uint8_t column;
} point_t;

//struct de una seccion de la pantalla usable por uno o más tasks
typedef struct screenState_t
{
    uint8_t screenId;    //identificador único de cada seccion de screen. Si es 0, es porque este screenState está vacío
    point_t topLeft;    //punto del límite superior izquierdo de la sección de la ventana
    point_t bottomRight;    //punto del límite inferior derecho de la sección de la ventana
    point_t cursor;    //posición actual del cursor de esta sección de la ventana
} screenState_t;

//Para ser llamadas desde el scheduler
int8_t addScreenState(uint8_t topLeftRow, uint8_t topLeftColumn, uint8_t bottomRightRow, uint8_t bottomRightColumn);

void deleteScreenState(uint8_t screenId);

//syscalls
uint8_t printChar(char character, const struct format_t *format);

char *print(const char *string, const struct format_t *format);

uint8_t newLine(color_t backgroundColor);

void clearScreen(color_t backgroundColor);

void getCursor(struct point_t *cursor);

void setCursor(const struct point_t *cursor);

void scrollUp(uint8_t rows);

void moveCursor(int row, int columns);

char *convert(unsigned int num, int base, unsigned int minDigitCount);

#endif

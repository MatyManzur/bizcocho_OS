// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <printing.h>

#define WIDTH 80
#define HEIGHT 25

enum specialFormat
{
    NONE = 0,
    CH_ONLY,
    BG_ONLY,
    BG_CH
};

static uint8_t *const video = (uint8_t *)0xB8000;

format_t currentFormat = {BLACK, L_GRAY};

uint8_t charsWithSpecialFormat[WIDTH][HEIGHT] = {{NONE}};

point_t cursor = {0, 0};

point_t backspaceBase = {0, 0};

// Funcion auxiliar que recibe un punto y devuelve un puntero a la dirección de memoria correspondiente a ese punto
static uint8_t *pointToCursor(point_t point)
{
    return (uint8_t *)(video + 2 * (point.row * WIDTH + point.column));
}

// Borra un char siempre que estemos después de la posicion del backspaceBase
static void backspace()
{

    if (pointToCursor(cursor) > pointToCursor(backspaceBase))
    {
        // nos movemos uno hacia atras
        if (cursor.column > 0)
        {
            cursor.column--;
        }
        else
        {
            cursor.row--;
            cursor.column = WIDTH - 1;
            while (*pointToCursor(cursor) == ' ' && cursor.column > 0)
            {
                cursor.column--;
            }
        }
        // ahora borramos el caracter
        uint8_t *cursorPointer = pointToCursor(cursor);
        charsWithSpecialFormat[cursor.column][cursor.row] = NONE;
        *cursorPointer = ' ';
        *(cursorPointer + 1) = currentFormat.backgroundColor << 4 | currentFormat.characterColor;
    }
}

void setBackspaceBase()
{
    backspaceBase.row = cursor.row;
    backspaceBase.column = cursor.column;
}

uint8_t printChar(char character, color_t backgroundColor, color_t characterColor)
{
    if (cursor.column >= WIDTH)
    {
        cursor.column = 0;
        cursor.row++;
    }
    if (cursor.row >= HEIGHT)
    {
        return 1; //se termino la pantalla
    }

    format_t fmt;
    fmt.backgroundColor = backgroundColor == DEFAULT ? currentFormat.backgroundColor : backgroundColor;
    fmt.characterColor = characterColor == DEFAULT ? currentFormat.characterColor : characterColor;

    if (character == '\n')
    {
        newLine();
        return 0;
    }
    if (character == '\b')
    {
        backspace();
        return 0;
    }

    uint8_t *cursorPointer = pointToCursor(cursor);
    charsWithSpecialFormat[cursor.column][cursor.row] = (backgroundColor != DEFAULT) << 1 | (characterColor != DEFAULT);
    *cursorPointer = (uint8_t)character;
    *(cursorPointer + 1) = fmt.backgroundColor << 4 | fmt.characterColor;

    cursor.column++;
    return 0;
}

// Printea el string en la pantalla. Si se termina la pantalla a mitad de camino, 
// le devuelve un puntero a la parte del string que quedó por printear
char *print(const char *string, const struct format_t *format)
{
    int i;
    for (i = 0; string[i] != 0; i++)
    {
        int error = printChar(string[i], format->backgroundColor, format->characterColor);
        if (error)
            return (char *)string + i;
    }
    return NULL;
}

format_t changeColor(color_t backgroundColor, color_t characterColor)
{
    format_t fmt = {DEFAULT, DEFAULT};
    if (backgroundColor < BLACK || backgroundColor > WHITE || characterColor < BLACK || characterColor > WHITE)
        return fmt;
    fmt.backgroundColor = currentFormat.backgroundColor;
    fmt.characterColor = currentFormat.characterColor;
    currentFormat.backgroundColor = backgroundColor;
    currentFormat.characterColor = characterColor;
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            point_t currentPoint = {.row = j, .column = i};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            color_t chColor = *(cursorPointer + 1) & 0x0F;
            color_t bgColor = *(cursorPointer + 1) & 0xF0;
            switch (charsWithSpecialFormat[currentPoint.column][currentPoint.row])
            {
            case BG_ONLY:
                chColor = characterColor;
                break;
            case CH_ONLY:
                bgColor = backgroundColor << 4;
                break;
            case NONE:
                chColor = characterColor;
                bgColor = backgroundColor << 4;
                break;
            default:
                break;
            }
            *(cursorPointer + 1) = bgColor | chColor;
        }
    }
    return fmt;
}

uint8_t newLine()
{
    while (cursor.column < WIDTH)
    {
        int error = printChar(' ', DEFAULT, DEFAULT); 
        if (error)
            return 1; 
    }
    cursor.column = 0;
    cursor.row++;
    return 0;
}

// borra los caracteres que haya en toda la screen
// deja el cursor al principio de todo
void clearScreen()
{
    for (int i = 0; i < WIDTH; i++)
    {
        for (int j = 0; j < HEIGHT; j++)
        {
            point_t currentPoint = {.row = j, .column = i};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            *cursorPointer = ' ';
            charsWithSpecialFormat[currentPoint.column][currentPoint.row] = NONE;
            *(cursorPointer + 1) = currentFormat.backgroundColor << 4 | currentFormat.characterColor;
        }
    }
    
    cursor.row = 0;
    cursor.column = 0;
    backspaceBase.row = 0;
    backspaceBase.column = 0;
}

// copia todas las lineas "rows" líneas hacia arriba. 
// las de abajo las borra para que no se vean repetidas con las que copio hacia arriba
void scrollUp(uint8_t rows)
{
    int j;
    for (j = 0; j < HEIGHT - rows; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            point_t currentPoint = {.row = j + rows, .column = i};
            point_t newPoint = {.row = j, .column = i};
            *(pointToCursor(newPoint)) = *(pointToCursor(currentPoint));
            *(pointToCursor(newPoint) + 1) = *(pointToCursor(currentPoint) + 1);
            charsWithSpecialFormat[newPoint.column][newPoint.row] = charsWithSpecialFormat[currentPoint.column][currentPoint.row];
        }
    }
    for (; j < HEIGHT; j++)
    {
        for (int i = 0; i < WIDTH; i++)
        {
            point_t currentPoint = {.row = j, .column = i};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            *(cursorPointer) = 0;
            *(cursorPointer + 1) = currentFormat.backgroundColor << 4 | currentFormat.characterColor;
            charsWithSpecialFormat[currentPoint.column][currentPoint.row] = NONE;
        }
    }
    cursor.column = 0;
    cursor.row = HEIGHT - rows;
    if (backspaceBase.row >= rows)
    {
        backspaceBase.row -= rows;
    }
    else
    {
        backspaceBase.row = 0;
        backspaceBase.column = 0;
    }
}
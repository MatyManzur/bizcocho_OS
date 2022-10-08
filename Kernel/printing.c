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

static uint8_t *const video = (uint8_t *) 0xB8000;

format_t currentFormat = {BLACK, L_GRAY};

uint8_t charsWithSpecialFormat[WIDTH][HEIGHT] = {NONE};

point_t cursor = {0,0};

//Funcion auxiliar que recibe un punto y devuelve un puntero a la dirección de memoria correspondiente a ese punto
static uint8_t *pointToCursor(point_t point)
{
    return (uint8_t * )(video + 2 * (point.row * WIDTH + point.column));
}

//printea el caracter indicado en donde se encuentre el cursor de la pantalla actual con el formato indicado
uint8_t printChar(char character, color_t backgroundColor, color_t characterColor)
{
    //si se pasó dentro de la linea, sigue al principio de la línea siguiente
    if (cursor.column > WIDTH)
    {
        cursor.column = 0;
        cursor.row++;
    }
    if (cursor.row > HEIGHT)
    {
        return 1; //se paso de su pantalla
    }

    format_t fmt;
    fmt.backgroundColor = backgroundColor == DEFAULT ? currentFormat.backgroundColor : backgroundColor;
    fmt.characterColor = characterColor == DEFAULT ? currentFormat.characterColor : characterColor;

    //escribimos el caracter con los colores indicados en la posicion de la pantalla indicada por el cursor de esta screenState
    uint8_t *cursorPointer = pointToCursor(cursor);
    charsWithSpecialFormat[cursor.column][cursor.row] = (backgroundColor != DEFAULT) << 1 | (characterColor != DEFAULT);
    *cursorPointer = (uint8_t) character;
    *(cursorPointer + 1) = fmt.backgroundColor << 4 | fmt.characterColor;

    //movemos el cursor a la siguiente columna para la proxima vez que printee algo
    cursor.column++;
    return 0;
}

//printea el string indicado con el formato indicado a partir de donde se encuentre el cursor de la pantalla actual. Si se termina la pantalla a mitad de camino, le devuelve un puntero a la parte del string que quedó por printear
char *print(const char *string, const struct format_t *format)
{
    int i;
    for (i = 0; string[i] != 0; i++)
    {
        int error = printChar(string[i], format->backgroundColor, format->characterColor);
        if (error)
            return (char*)string + i;
    }
    return NULL;
}

format_t changeColor(color_t backgroundColor, color_t characterColor)
{
    format_t fmt = {DEFAULT, DEFAULT};
    if(backgroundColor < BLACK || backgroundColor > WHITE || characterColor < BLACK || characterColor > WHITE)
        return fmt;
    fmt.backgroundColor = currentFormat.backgroundColor;
    fmt.characterColor = currentFormat.characterColor;
    currentFormat.backgroundColor = backgroundColor;
    currentFormat.characterColor = characterColor;
    for (int i = 0; i <= WIDTH; i++)
    {
        for (int j = 0; j <= HEIGHT; j++)
        {
            point_t currentPoint = {.row = j, .column = i};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            color_t chColor = *(cursorPointer + 1) & 0x0F;
            color_t bgColor = *(cursorPointer + 1) & 0xF0;
            switch(charsWithSpecialFormat[currentPoint.column][currentPoint.row])
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

//borra los caracteres que queden en la línea y setea el fondo al color indicado
uint8_t newLine(color_t backgroundColor)
{
    while (cursor.column <= WIDTH)
    {
        int error = printChar(' ', backgroundColor, DEFAULT);    //hacemos esto hasta que lleguemos al fin de la línea
        if (error)
            return 1; //si se paso de la pantalla
    }
    //y movemos el cursor al principio de la línea siguiente
    cursor.column = 0;
    cursor.row++;
    return 0;
}


//borra los caracteres que haya en toda la screen
//deja el cursor al principio de todo
void clearScreen()
{
    for (int i = 0; i <= WIDTH; i++)
    {
        for (int j = 0; j <= HEIGHT; j++)
        {
            point_t currentPoint = {.row = j, .column = i};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            *cursorPointer = ' ';
            charsWithSpecialFormat[currentPoint.column][currentPoint.row] = NONE;
            *(cursorPointer + 1) = currentFormat.backgroundColor << 4 | currentFormat.characterColor;
        }
    }
    //pone el cursor al principio
    cursor.row = 0;
    cursor.column = 0;
}

//copia todas las lineas "rows" líneas hacia arriba. las de abajo las borra para que no se vean repetidas con las que copio hacia arriba
void scrollUp(uint8_t rows)
{
    int j;
    for (j = 0; j <= HEIGHT - rows; j++)
    {
        for (int i = 0; i <= WIDTH; i++)
        {
            point_t currentPoint = {.row = j + rows, .column = i};
            point_t newPoint = {.row = j, .column = i};
            *(pointToCursor(newPoint)) = *(pointToCursor(currentPoint));
            *(pointToCursor(newPoint) + 1) = *(pointToCursor(currentPoint) + 1);
            charsWithSpecialFormat[newPoint.column][newPoint.row] = charsWithSpecialFormat[currentPoint.column][currentPoint.row];
        }
    }
    //borra los caracteres de las úlimas "rows" líneas
    for (; j <= HEIGHT; j++)
    {
        for (int i = 0; i <= WIDTH; i++)
        {
            point_t currentPoint = {.row = i, .column = j};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            *(cursorPointer) = 0;
            *(cursorPointer + 1) = currentFormat.backgroundColor << 4 | currentFormat.characterColor;
            charsWithSpecialFormat[currentPoint.column][currentPoint.row] = NONE;
        }
    }
}
#include <printing.h>


static uint8_t *const video = (uint8_t *) 0xB8000;
static const uint32_t width = 80;
static const uint32_t height = 25;

//Tenemos un array de secciones de pantalla (screenStates). Cada task usará una de estas screenStates.
static screenState_t screenStates[MAX_SCREEN_COUNT];

static uint8_t nextScreenId = 1; //screenIds a asignar. Arranca en 1 porque el 0 se usa para ver si el slot del array esta vacío

//Funcion auxiliar que recibe un punto y devuelve un puntero a la dirección de memoria correspondiente a ese punto
static uint8_t *pointToCursor(point_t point)
{
    return (uint8_t * )(video + 2 * (point.row * width + point.column));
}

//Funcion auxiliar que recibe un screenId lo busca en el array y devuelve el índice de donde lo encontró. Si no lo encuentra, devuelve -1
static int8_t getScreenArrayIndex(uint8_t screenId)
{
    for (int i = 0; i < MAX_SCREEN_COUNT; i++)
    {
        if (screenStates[i].screenId == screenId)
            return i;
    }
    return -1;
}

//Funcion que devuelve el indice en el array de la screen que usa la task actual, si hubo un error devuelve -1
static int8_t getCurrentScreenIndex()
{
    int8_t screenId = getCurrentScreenId();
    if (screenId < 0)
        return -1; //no hay una current task

    return getScreenArrayIndex(screenId);
}

//Agrega un screenState al array y devuelve su screenId
int8_t addScreenState(uint8_t topLeftRow, uint8_t topLeftColumn, uint8_t bottomRightRow, uint8_t bottomRightColumn)
{
    int8_t screenId = nextScreenId++; //generamos el screenId

    //Buscamos el próximo lugar del array sin usar
    int freeSlot = 0;
    while (freeSlot < MAX_SCREEN_COUNT && screenStates[freeSlot].screenId != 0)
        freeSlot++;
    if (freeSlot >= MAX_SCREEN_COUNT)    //si no encontró lugares libres devuelve -1
        return -1;

    //si nos pasó posiciones por fuera de la pantalla, las corregimos a adentro de la pantalla
    if (topLeftRow >= height)
        topLeftRow = height - 1;
    if (topLeftColumn >= width)
        topLeftColumn = width - 1;

    if (bottomRightRow >= height)
        bottomRightRow = height - 1;
    if (bottomRightColumn >= width)
        bottomRightColumn = width - 1;

    //creamos los puntos de los bordes de la pantalla
    point_t topLeft = {.row = topLeftRow, .column = topLeftColumn};
    point_t bottomRight = {.row = bottomRightRow, .column = bottomRightColumn};

    //agregamos el screenState al espacio libre encontrado del array
    screenStates[freeSlot] = (screenState_t) {.screenId = screenId, .topLeft = topLeft, .bottomRight = bottomRight, .cursor = topLeft};
    return screenId;
}

//Borra el screenState de la screenId indicada
void deleteScreenState(uint8_t screenId)
{
    int8_t screenIndex = getScreenArrayIndex(screenId);
    if (screenIndex < 0)
        return;    //si no había screens con esa screenId no hace nada
    //si lo encontró, lo borra
    screenStates[screenIndex] = (screenState_t) {0};
}

//printea el caracter indicado en donde se encuentre el cursor de la pantalla actual con el formato indicado
uint8_t printChar(char character, const struct format_t *format)
{
    //nos fijamos la screen actual
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return 2; //hubo un error al obtener la current screen

    //si se pasó dentro de la linea, sigue al principio de la línea siguiente
    if (screenStates[screenIndex].cursor.column > screenStates[screenIndex].bottomRight.column)
    {
        screenStates[screenIndex].cursor.column = screenStates[screenIndex].topLeft.column;
        screenStates[screenIndex].cursor.row++;
    }
    if (screenStates[screenIndex].cursor.row > screenStates[screenIndex].bottomRight.row)
    {
        return 1; //se paso de su pantalla
    }

    //escribimos el caracter con los colores indicados en la posicion de la pantalla indicada por el cursor de esta screenState
    uint8_t *cursorPointer = pointToCursor(screenStates[screenIndex].cursor);
    *cursorPointer = (uint8_t) character;
    *(cursorPointer + 1) = format->backgroundColor << 4 | format->characterColor;

    //movemos el cursor a la siguiente columna para la proxima vez que printee algo
    screenStates[screenIndex].cursor.column++;
    return 0;
}

//printea el string indicado con el formato indicado a partir de donde se encuentre el cursor de la pantalla actual. Si se termina la pantalla a mitad de camino, le devuelve un puntero a la parte del string que quedó por printear
char *print(const char *string, const struct format_t *format)
{
    int i;
    for (i = 0; string[i] != 0; i++)
    {
        int error = printChar(string[i], format);
        if (error)
            return string + i;
    }
    return NULL;
}

//borra los caracteres que haya en la línea y setea el fondo al color indicado
uint8_t newLine(color_t backgroundColor)
{
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return 2; //hubo un error al obtener la current screen

    format_t format = {.backgroundColor = backgroundColor, .characterColor = 0};
    while (screenStates[screenIndex].cursor.column <= screenStates[screenIndex].bottomRight.column)
    {
        int error = printChar(' ', &format);    //hacemos esto hasta que lleguemos al fin de la línea
        if (error)
            return 1; //si se paso de la pantalla
    }
    //y movemos el cursor al principio de la línea siguiente
    screenStates[screenIndex].cursor.column = screenStates[screenIndex].topLeft.column;
    screenStates[screenIndex].cursor.row++;
    return 0;
}


//borra los caracteres que haya en toda la screen de este screenState y setea el fondo al color indicado
//deja el cursor al principio de todo dentro de este screenState
void clearScreen(color_t backgroundColor)
{
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return; //hubo un error al obtener la current screen

    for (int i = screenStates[screenIndex].topLeft.row; i <= screenStates[screenIndex].bottomRight.row; i++)
    {
        for (int j = screenStates[screenIndex].topLeft.column; j <= screenStates[screenIndex].bottomRight.column; j++)
        {
            point_t currentPoint = {.row = i, .column = j};
            uint8_t *cursorPointer = pointToCursor(currentPoint);
            *cursorPointer = ' ';
            *(cursorPointer + 1) = backgroundColor << 4;
        }
    }

    //pone el cursor al principio
    screenStates[screenIndex].cursor = screenStates[screenIndex].topLeft;
}

//devuelve por parametro la posicion del cursor de la screenState actual, por si userland se la quiere guardar para restaurarla despues
void getCursor(struct point_t *cursor)
{
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return; //hubo un error al obtener la current screen

    cursor->row = screenStates[screenIndex].cursor.row;
    cursor->column = screenStates[screenIndex].cursor.column;
}

//setea el cursor de la screenState actual al indicado por parámetro, si está por fuera de topLeft lo corrige acá. si está por fuera de bottomRight se va a dar cuenta el próximo printChar y lo va a corregir
void setCursor(const struct point_t *cursor)
{
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return; //hubo un error al obtener la current screen

    screenStates[screenIndex].cursor.row = (cursor->row < screenStates[screenIndex].topLeft.row)
                                           ? screenStates[screenIndex].topLeft.row : cursor->row;
    screenStates[screenIndex].cursor.column = (cursor->column < screenStates[screenIndex].topLeft.column)
                                              ? screenStates[screenIndex].topLeft.column : cursor->column;
}

//copia todas las lineas de la screenState actual "rows" líneas hacia arriba. las de abajo las borra para que no se vean repetidas con las que copio hacia arriba
void scrollUp(uint8_t rows)
{
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return; //hubo un error al obtener la current screen

    int i;
    //copiamos todas las líneas desde topLeft.row + rows hasta bottomRight.row "rows" líneas hacia arriba
    for (i = screenStates[screenIndex].topLeft.row; i <= screenStates[screenIndex].bottomRight.row - rows; i++)
    {
        for (int j = screenStates[screenIndex].topLeft.column; j <= screenStates[screenIndex].bottomRight.column; j++)
        {
            point_t currentPoint = {.row = i + rows, .column = j};
            point_t newPoint = {.row = i, .column = j};
            *(pointToCursor(newPoint)) = *(pointToCursor(currentPoint));
            *(pointToCursor(newPoint) + 1) = *(pointToCursor(currentPoint) + 1);
        }
    }
    //borra los caracteres de las úlimas "rows" líneas
    for (; i <= screenStates[screenIndex].bottomRight.row; i++)
    {
        for (int j = screenStates[screenIndex].topLeft.column; j <= screenStates[screenIndex].bottomRight.column; j++)
        {
            point_t currentPoint = {.row = i, .column = j};
            *(pointToCursor(currentPoint)) = 0;
        }
    }
}

//mueve el cursor de la screenState actual relativamente las rows y columns indicadas.
//rows < 0 => hacia arriba
//rows > 0 => hacia abajo
//columns < 0 => hacia la izquierda
//columns > 0 => hacia la derecha
void moveCursor(int rows, int columns)
{
    int8_t screenIndex = getCurrentScreenIndex();
    if (screenIndex < 0)
        return; //hubo un error al obtener la current screen

    //calculamos a donde tiene que ir
    int finalRow = ((int) screenStates[screenIndex].cursor.row) + rows;
    int finalColumn = ((int) screenStates[screenIndex].cursor.column) + columns;

    //si se pasa de los bordes lo corregimos
    if (finalRow > screenStates[screenIndex].bottomRight.row)
    {
        finalRow = screenStates[screenIndex].bottomRight.row;
    } else if (finalRow < screenStates[screenIndex].topLeft.row)
    {
        finalRow = screenStates[screenIndex].topLeft.row;
    }

    if (finalColumn > screenStates[screenIndex].bottomRight.column)
    {
        finalColumn = screenStates[screenIndex].bottomRight.column;
    } else if (finalColumn < screenStates[screenIndex].topLeft.column)
    {
        finalColumn = screenStates[screenIndex].topLeft.column;
    }

    //finalmente movemos el cursor a la posicion indicada
    screenStates[screenIndex].cursor.row = finalRow;
    screenStates[screenIndex].cursor.column = finalColumn;
}


char *convert(unsigned int num, int base, unsigned int minDigitCount)
{
    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50];
    char *ptr;
    int digitCount = 0;

    ptr = &buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = Representation[num % base];
        num /= base;
        digitCount++;
    } while (num != 0);

    while (digitCount < minDigitCount) //agrega ceros adelante si faltan digits
    {
        *--ptr = Representation[0];
        digitCount++;
    }

    return (ptr);
}







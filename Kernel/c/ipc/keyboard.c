#include <keyboard.h>

static char printableKeys[2][128] =    //mapa de caracteres printeables para cada tecla no especial de la distribucion US en su versión normal y shifteada
        {//Default US keys
                {
                        0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
                        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
                        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
                        'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
                        0, 0, 0, '+', 0
                },//Shifted US Keys
                {
                        0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
                        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
                        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',  'Z',
                        'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0,
                        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
                        0, 0, 0, '+', 0
                }};

static kbEvent_t buffer[BUFFER_DIM];    //tenemos un buffer de structs guardando las acciones con las teclas
static uint32_t readingIndex;    //indice del buffer que apunta al proximo a leer
static uint32_t writingIndex;    //indice del buffer que apunta al proximo a escribir

static uint8_t shifted;    //flag que se prende si el Shift está apretado
static uint8_t ctrld;    //flag que se prende si el Ctrl está apretado

//se llama cada vez que interrumpe el teclado, escribe en el buffer
void keyboard_handler()
{
    static int specialKey;    //flag que se prende si se apretó una tecla especial. Por ejemplo, las flechas, que interrumpen dos veces: una con un scanCode de E0, y la siguiente con su scancode particular.

    int scanCode = getScanCode();    //función en libasm.asm, pregunta al teclado el ultimo scancode

    //si fue un E0, es una tecla especial. Prendemos el flag de specialKey, y terminamos. Esperamos que el teclado vuelva a interrumpir con el siguiente scancode para identificar la tecla
    if (scanCode == 0xE0)
    {
        specialKey = 1;
        return;
    }

    //si estabamos esperando el segundo scancode de una tecla especial
    if (specialKey)
    {
        specialKey = 0;    //apagamos el flag de specialKey
        scanCode = -scanCode;    //y lo guardamos pero como un numero negativo (para diferenciarlo de los codigos no especiales)
    }

    //vamos a formar el struct a guardar con la tecla y la acción (apretar o soltar):
    kbEvent_t kbEvent;

    if (scanCode > 0x80)    //si el scanCode es > 0x80 es porque fue una accion de soltar una tecla no especial
    {
        scanCode -= 0x80;    // restamos 0x80 y obtenemos el scancode de la tecla
        kbEvent.action = RELEASED;    //y nos guardamos que fue una accion de soltar
    } else if (scanCode < -0x80)    //si el scanCode es < -0x80 fue una accion de soltar una tecla especial
    {
        scanCode += 0x80;    //sumamos 0x80 para obtener el scancode de la tecla especial
        kbEvent.action = RELEASED;    //y nos guardamos que fue una accion de soltar
    } else    //si no, es porque fue una accion de apretar (tecla especial o no)
    {
        kbEvent.action = PRESSED;
    }
    kbEvent.key = scanCode;    //nos guardamos el scancode ya transformado
    if (scanCode == VK_F1)    //con F1 nos guardamos una screenshot de los registros
    {
        saveRegisters(); 
    }
    if (scanCode == VK_ESCAPE && kbEvent.action == RELEASED)
    {
        killAllInDeathList();
    }
    buffer[(writingIndex++) % BUFFER_DIM] = kbEvent;    //guardamos el struct en el buffer en la posicion indicada por writingIndex%BUFFER_DIM y lo incrementamos
    
    BlockedReason_t block;
    block.id=0;
    block.source=PIPE_READ;
    unblockAllProcessesBecauseReason(block);
}

/*
Intenta leer del buffer "count" caracteres PRINTEABLES (omite teclas no printeables) 
y los guarda en el buffer pasado por parametro. 
Corta si encuentra un \n o un Ctrl+D
ya transforma las mayúsculas y las pone como un caracter en el buffer pasado por parametro.
Devuelve los caracteres que alcanzó a leer.
*/
uint8_t readPrintables(char *bufferString, uint8_t count)
{
    int i = 0; //contamos caracteres enteros leidos
    while (i < count)
    {
        if (readingIndex < writingIndex)    //si hay algo para leer
        {
            //leemos el evento del buffer
            kbEvent_t kbEvent = buffer[(readingIndex++) % BUFFER_DIM];
            //si fue alguno de los dos shifts cambiamos el flag de shifted
            if (kbEvent.key == VK_LSHIFT || kbEvent.key == VK_RSHIFT)
            {
                shifted = !shifted;
            }
                //o si se apreto el Bloc Mayusc tambien
            else if (kbEvent.key == VK_CAPITAL && kbEvent.action == PRESSED)
            {
                shifted = !shifted;
            }
            else if (kbEvent.key == VK_LCONTROL || kbEvent.key == VK_RCONTROL)
            {
                ctrld = !ctrld;
            }
                //si no fue algo de shift y no fue una tecla especial
            else if (kbEvent.key > 0 && kbEvent.action == PRESSED)
            {
                if(ctrld && kbEvent.key == VK_D) //Si apretan Ctrl+D manda un \0
                {
                    bufferString[i++] = '\0';
                    return i;
                }
                //obtenemos el caracter correspondiente
                char printableChar = printableKeys[shifted][kbEvent.key];
                //si esa tecla tenía un caracter printeable, lo ponemos en el string e i++
                if (printableChar != 0)
                {
                    bufferString[i] = printableChar;
                    if(bufferString[i++] == '\n')
                        return i;
                }
                //si no era un caracter printeable, no hacemos nada y no incrementamos i
            }
        } else    //si todavía no hay nada para leer
        {
            return i;
        }
    }
    return i;
}

//lleva el readingIndex hasta el writingIndex pero fijandose qué paso con las teclas de shift
void cleanBuffer()
{
    while (readingIndex < writingIndex)
    {
        kbEvent_t kbEvent = buffer[(readingIndex++) % BUFFER_DIM];
        if (kbEvent.key == VK_LSHIFT || kbEvent.key == VK_RSHIFT)
        {
            if (kbEvent.action == PRESSED)
                shifted = 1;
            else
                shifted = 0;
        } 
        else if (kbEvent.key == VK_CAPITAL && kbEvent.action == PRESSED)
        {
            shifted = !shifted;
        }
        else if (kbEvent.key == VK_LCONTROL || kbEvent.key == VK_RCONTROL)
        {
            if (kbEvent.action == PRESSED)
                ctrld = 1;
            else
                ctrld = 0;
        }
    }
}
#include <keyboard.h>


static kbEvent_t buffer[BUFFER_DIM];    //tenemos un buffer de structs guardando las acciones con las teclas
static uint32_t readingIndex;    //indice del buffer que apunta al proximo a leer
static uint32_t writingIndex;    //indice del buffer que apunta al proximo a escribir

static uint8_t shifted;    //flag que se prende si el Shift está apretado

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
    buffer[(writingIndex++) %
           BUFFER_DIM] = kbEvent;    //guardamos el struct en el buffer en la posicion indicada por writingIndex%BUFFER_DIM y lo incrementamos
}

//lee del buffer "count" caracteres PRINTEABLES (omite teclas no printeables) y los guarda en el buffer pasado por parametro. Empieza a leer del teclado en cuanto se llama a esta funcion y espera a que se hayan leído la cantidad deseada de caracteres, ya transforma las mayúsculas y las pone como un caracter en el buffer pasado por parametro.
void readPrintables(char *bufferString, uint8_t count)
{
    cleanBuffer(); //omitimos lo que haya quedado en el buffer de antes
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
                //si no fue algo de shift y no fue una tecla especial
            else if (kbEvent.key > 0 && kbEvent.action == PRESSED)
            {
                //obtenemos el caracter correspondiente
                char printableChar = printableKeys[shifted][kbEvent.key];
                //si esa tecla tenía un caracter printeable, lo ponemos en el string e i++
                if (printableChar != 0)
                    bufferString[i++] = printableChar;
                //si no era un caracter printeable, no hacemos nada y no incrementamos i
            }
        } else    //si todavía no hay nada para leer
        {
            _hlt();    //esperamos a la próxima interrupción
        }
    }
}

//Lee la próxima tecla del buffer en la posición indicada por readingIndex, y lo incrementa siempre que haya habido algo para leer. Lee teclas printeables y no printeables.
void getNextKey(struct kbEvent_t *kbEvent)
{
    if (readingIndex == writingIndex) //si no hay nada para leer, devuelve key=0. No espera a que se aprete algo
    {
        kbEvent->key = 0;
        return;
    }
    kbEvent->key = buffer[(readingIndex) % BUFFER_DIM].key;
    kbEvent->action = buffer[(readingIndex) % BUFFER_DIM].action;
    readingIndex++;
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
        } else if (kbEvent.key == VK_CAPITAL && kbEvent.action == PRESSED)
        {
            shifted = !shifted;
        }
    }
}

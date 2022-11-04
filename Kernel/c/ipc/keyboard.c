// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <keyboard.h>

static char printableKeys[2][128] = 
    {                               // Default US keys
        {
            0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
            '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z',
            'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' ', 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
            0, 0, 0, '+', 0}, // Shifted US Keys
        {
            0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
            '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,
            'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|', 'Z',
            'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-',
            0, 0, 0, '+', 0}};

static kbEvent_t buffer[BUFFER_DIM]; 
static uint32_t readingIndex;        
static uint32_t writingIndex;        

static uint8_t shifted; 
static uint8_t ctrld;   

// se llama cada vez que interrumpe el teclado, escribe en el buffer
void keyboard_handler()
{
    static int specialKey; // flag que se prende si se apretó una tecla especial. Por ejemplo, las flechas, que interrumpen dos veces: una con un scanCode de E0, y la siguiente con su scancode particular.

    int scanCode = getScanCode(); 

    if (scanCode == 0xE0)
    {
        specialKey = 1;
        return;
    }

    if (specialKey)
    {
        specialKey = 0;       
        scanCode = -scanCode; // lo guardamos como un numero negativo (para diferenciarlo de los codigos no especiales)
    }

    kbEvent_t kbEvent;

    if (scanCode > 0x80) // si el scanCode es > 0x80 es porque fue una accion de soltar una tecla no especial
    {
        scanCode -= 0x80;          
        kbEvent.action = RELEASED; 
    }
    else if (scanCode < -0x80) // si el scanCode es < -0x80 fue una accion de soltar una tecla especial
    {
        scanCode += 0x80;          
        kbEvent.action = RELEASED; 
    }
    else // si no, es porque fue una accion de apretar (tecla especial o no)
    {
        kbEvent.action = PRESSED;
    }
    kbEvent.key = scanCode; 
    if (scanCode == VK_F1)  
    {
        saveRegisters();
    }
    if (scanCode == VK_ESCAPE && kbEvent.action == RELEASED)
    {
        killAllInDeathList();
    }
    buffer[(writingIndex++) % BUFFER_DIM] = kbEvent; 

    BlockedReason_t block;
    block.id = 0;
    block.source = PIPE_READ;
    unblockAllProcessesBecauseReason(block);
}

/*
Intenta leer del buffer "count" caracteres PRINTEABLES (omite teclas no printeables)
y los guarda en el buffer pasado por parametro.
Corta si encuentra un \n o un Ctrl+D
ya transforma las mayúsculas y las pone como un caracter en el buffer pasado por parametro.
Devuelve los caracteres que alcanzó a leer. No bloquea.
*/
uint8_t readPrintables(char *bufferString, uint8_t count)
{
    int i = 0; 
    while (i < count)
    {
        if (readingIndex < writingIndex) 
        {
            kbEvent_t kbEvent = buffer[(readingIndex++) % BUFFER_DIM];
            
            if (kbEvent.key == VK_LSHIFT || kbEvent.key == VK_RSHIFT)
            {
                shifted = !shifted;
            }
            
            else if (kbEvent.key == VK_CAPITAL && kbEvent.action == PRESSED)
            {
                shifted = !shifted;
            }
            else if (kbEvent.key == VK_LCONTROL || kbEvent.key == VK_RCONTROL)
            {
                ctrld = !ctrld;
            }
            
            else if (kbEvent.key > 0 && kbEvent.action == PRESSED)
            {
                if (ctrld && kbEvent.key == VK_D) // Si apretan Ctrl+D manda un \0
                {
                    bufferString[i++] = '\0';
                    return i;
                }
                
                char printableChar = printableKeys[shifted][kbEvent.key];
                
                if (printableChar != 0)
                {
                    bufferString[i] = printableChar;
                    if (bufferString[i++] == '\n')
                        return i;
                }
                
            }
        }
        else // si todavía no hay nada para leer
        {
            return i;
        }
    }
    return i;
}

// lleva el readingIndex hasta el writingIndex pero fijandose qué paso con las teclas de shift
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

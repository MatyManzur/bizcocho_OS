#include <bizcocho.h>

#define LASTCOLUMN 80
#define LASTLINE 25
#define COMMAND_COUNT 8
#define COLOROPTIONS 3

#define BUFFER_DIM 50

#define WIDTH 80
#define HEIGHT 25

#define MAX_LONG_TOKEN_LENGTH 30
#define MAX_TOKEN_LENGTH 20
#define MAX_ARG_COUNT 4

void printMonkey();

void printLogo();

void addMessage(const char *message);

int changeColor(const char *buffer, const char *colors[], color_t colorValues[]);

static point_t promptCursor = {HEIGHT - 1, 0};
static point_t printingCursor = {0, 0};

typedef struct command_t
{
    char *name; //el string que tiene que leer del prompt
    uint8_t runnable;

    void (*programFunction)(uint8_t argc, void **argv); //a quien tiene que llamar
} command_t;

static command_t commands[COMMAND_COUNT] = {
        {.name="help", .runnable = 0, .programFunction = help},
        {.name="inforeg", .runnable = 0, .programFunction = printRegisters},
        {.name="time", .runnable = 0, .programFunction = time},
        {.name="fibonacci", .runnable = 1, .programFunction = fibonacci},
        {.name="prime", .runnable = 1, .programFunction = prime},
        {.name="divZero", .runnable = 0, .programFunction = divZero},
        {.name="invalidOpcode", .runnable = 0, .programFunction = invalidOpcode},
        {.name="printmem", .runnable = 0, .programFunction = printmem}
};

static color_t colorValues[COLOROPTIONS] = {L_GRAY, BLACK, MAGENTA};
static const char *colors[COLOROPTIONS] = {"letter",
                                           "background",
                                           "user"};


void bizcocho(uint8_t argc, void **argv)
{

    char promptBuffer[BUFFER_DIM] = {0};

    sys_clear_screen(colorValues[1]);

    char start = 0;
    setColor(BLACK, L_CYAN);
    sys_set_cursor(&printingCursor);
    for (int i = 0; i < 5; i++)
    {
        newLine();
    }
    sys_get_cursor(&printingCursor);
    printLogo();
    for (int i = 0; i < 5; i++)
    {
        newLine();
    }
    printString("                                Bienvenidos! \2");
    newLine();
    printString("                    Presione la tecla espacio para comenzar...");
    newLine();
    printString("              ( Luego escriba \"help\" para conocer comandos y teclas )");

    while (start != ' ')
    {
        sys_read_printables(&start, 1);
    }

    sys_clear_screen(colorValues[1]);
    sys_get_cursor(&printingCursor);

    //set cursor al inicio de todo
    while (1)
    {
        if (printingCursor.row >= HEIGHT || (printingCursor.row == HEIGHT - 1 && printingCursor.column >
                                                                                 0)) //esta 2da opcion es por si el programa no tiene un newline al final
        {
            sys_set_cursor(&printingCursor);
            sys_move_cursor(-1, 0);
            sys_get_cursor(&printingCursor);
            sys_scroll_up(1);
        }

        sys_set_cursor(&promptCursor);    //Reseteamos la linea del prompt

        sys_new_line(colorValues[1]);

        sys_set_cursor(&promptCursor);

        setColor(colorValues[1], colorValues[2]);
        printString("Bizcocho ");
        putChar(2);
        putChar(' ');
        putChar(16); //para el chirimbolito

        setColor(colorValues[1], colorValues[0]);

        char key = 0;
        int counter = 0; //cuantas letras van en este mensaje

        do
        { //repite hasta un enter o que hayan BUFFER_DIM letras
            sys_read_printables(&key, 1); //leemos la letra y la dejamos en key

            if (key != '\n') //si es un enter, no printeamos nada y va a salir del while
            {
                if (key != '\b')
                { //si no es un backspace
                    promptBuffer[counter++] = key; //la ponemos en el promptBuffer
                    putChar(key); // printeamos la key, porq sino no va a aparecer hasta que apretemos enter
                } else if (counter > 0)
                { //si se apreto backspace y no habia nada para borrar no hace nada
                    promptBuffer[--counter] = '\0'; //borramos del buffer

                    sys_move_cursor(0, -1);    //se mueve uno para atras para borrarlo

                    //y printeamos un vacio
                    putChar(' ');

                    sys_move_cursor(0, -1);    //se mueve uno para atras para reemplazar lo que se borro
                }
            }

        } while (key != '\n' && counter < BUFFER_DIM);

        promptBuffer[counter] = '\0';

        addMessage(promptBuffer);

        //en promptBuffer está todo lo que lee del prompt cuando se apreto enter

        char pipeTokens[2][MAX_LONG_TOKEN_LENGTH] = {0};

        int pipeTokensCount = parser(promptBuffer, &pipeTokens, '|', 2, MAX_LONG_TOKEN_LENGTH);

        //en pipeTokens[0] está lo que esté a la izquierda del '|', o todo el string en caso de que no haya '|'
        //en pipeTokens[1] está lo que esté a la derecha del '|' en caso de que haya pipe
        //pipeTokensCount = 1 si no hay pipe, = 2 si hay pipe

        int index1, index2;
        int foundFlag = 0;
        char *argString1;
        char *argString2;
        int argc1, argc2;
        char *argv1[MAX_ARG_COUNT] = {NULL};
        char *argv2[MAX_ARG_COUNT] = {NULL};
        char firstTokens[MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {0};
        char secondTokens[MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {0};

        //buscamos comando en pipeTokens[0]
        for (int i = 0; i < COMMAND_COUNT && !foundFlag; i++)
        {
            if (strPrefix(commands[i].name, pipeTokens[0], &argString1))
            {
                foundFlag = 1;
                index1 = i;
            }
        }

        //si reconocemos un comando, nos guardamos los argumentos que haya pasado
        if (foundFlag)
        {
            argc1 = parser(argString1, &firstTokens, ' ', MAX_ARG_COUNT, MAX_TOKEN_LENGTH);
            for (int i = 0; i < argc1; i++)
            {
                argv1[i] = firstTokens[i];
            }
        }

        //si reconocimos el primer comando y había pipe, buscamos reconocer el segundo comando y sus argumentos
        if (foundFlag && pipeTokensCount == 2)
        {
            for (int i = 0; i < COMMAND_COUNT && foundFlag == 1; i++)
            {
                if (strPrefix(commands[i].name, pipeTokens[1], &argString2))
                {
                    foundFlag = 2;
                    index2 = i;
                }
            }

            if (foundFlag == 2)
            {
                argc2 = parser(argString2, &secondTokens, ' ', MAX_ARG_COUNT, MAX_TOKEN_LENGTH);
                for (int i = 0; i < argc2; i++)
                {
                    argv2[i] = secondTokens[i];
                }
            } else
            {
                foundFlag = 0;
            }
        }

        if (!foundFlag && pipeTokensCount == 1) //si no hubo pipe y no reconcio un comando puede ser un changeColor
        {
            if (strPrefix(pipeTokens[0], "monkey", NULL))
            {
                printMonkey();
                foundFlag = -1;
            } else if (strPrefix(pipeTokens[0], "clear", NULL))
            {
                sys_clear_screen(colorValues[1]);
                foundFlag = -1;
                sys_get_cursor(&printingCursor);
            } else
            {
                int isChangeColor = changeColor(pipeTokens[0], colors, colorValues);
                if (isChangeColor)
                {
                    addMessage("Color changed!");
                    foundFlag = -1;
                }
            }
        }

        if (!foundFlag) //si no fue un comando ni cambiar color, damos error
        {
            addMessage("Hey! That's not a valid command!");
        } else if (foundFlag > 0) //si tenemos que correr un comando
        {
            sys_set_cursor(&printingCursor);

            int bizcochoId = sys_get_task_id();

            if (pipeTokensCount <= 1) //no hay pipe
            {
                if (commands[index1].runnable) //si necesita el runner
                {
                    functionPointer_t function = {commands[index1].programFunction};
                    void *args[3] = {&function, &argc1, &argv1};
                    sys_add_task_with_shared_screen(runner, bizcochoId, 0, 3, args);
                } else    //si corre solo, sin el runner
                {
                    sys_add_task_with_shared_screen(commands[index1].programFunction, bizcochoId, 0, argc1, argv1);
                }
            } else //hay pipe
            {
                functionPointer_t function1 = {commands[index1].programFunction};
                functionPointer_t function2 = {commands[index2].programFunction};
                void *args[6] = {&function1, &argc1, argv1, &function2, &argc2, argv2};

                sys_add_task_with_shared_screen(runner, bizcochoId, 0, 6, args);
            }

            sys_deactivate_task(bizcochoId);

            sys_get_cursor(&printingCursor);
        }

    }
}

//printea el string indicado en la terminal y pone un enter al final
void addMessage(const char *message)
{
    sys_set_cursor(&printingCursor);
    printStringColor(message, colorValues[1], colorValues[0]);
    sys_new_line(colorValues[1]);
    sys_get_cursor(&printingCursor);
}


int changeColor(const char *buffer, const char *colors[], color_t colorValues[])
{
    for (int i = 0; i < COLOROPTIONS; i++)
    {
        if (strPrefix(colors[i], buffer, NULL))
        {
            char aux = 0x00;
            if ((aux = strToNum(buffer + strLength(colors[i]) + 1)) <= 15 && aux >= 0)
            {
                if (i == 1)
                {
                    sys_clear_screen(aux);
                    sys_get_cursor(&printingCursor);
                }
                colorValues[i] = aux;
                return 1;
            } else
            {
                return 0;
            }
        }
    }
    if (strCmp(buffer, "boquita") == 0)
    {
        colorValues[0] = YELLOW;
        colorValues[1] = BLUE;
        colorValues[2] = YELLOW;
    } else if (strCmp(buffer, "river") == 0)
    {
        colorValues[0] = RED;
        colorValues[1] = WHITE;
        colorValues[2] = RED;
    } else if (strCmp(buffer, "banfield") == 0)
    {
        colorValues[0] = WHITE;
        colorValues[1] = GREEN;
        colorValues[2] = WHITE;
    } else
    {
        return 0;
    }
    sys_clear_screen(colorValues[1]);
    sys_get_cursor(&printingCursor);
    return 1;
}

//https://www.asciiart.eu/animals/monkeys
void printMonkey()
{
    sys_set_cursor(&printingCursor);
    printStringColor("       .-\"-.            .-\"-.            .-\"-.           .-\"-.", GREEN, YELLOW);
    sys_new_line(GREEN);
    printStringColor("     _/_-.-_\\_        _/.-.-.\\_        _/.-.-.\\_       _/.-.-.\\_", GREEN, YELLOW);
    sys_new_line(GREEN);
    printStringColor("    / __} {__ \\      /|( o o )|\\      ( ( o o ) )     ( ( o o ) )", GREEN, YELLOW);
    sys_new_line(GREEN);
    printStringColor("   / //  \"  \\ \\     | //  \"  \\ |       |/  \"  \\|       |/  \"  \\|", GREEN, YELLOW);
    sys_new_line(GREEN);
    printStringColor("  / / \\'---'/ \\ \\  / / \\'---'/ \\ \\      \\'/^\\'/         \\ .-. /", GREEN, YELLOW);
    sys_new_line(GREEN);
    printStringColor("  \\ \\_/`\"\"\"`\\_/ /  \\ \\_/`\"\"\"`\\_/ /      /`\\ /`\\         /`\"\"\"`\\", GREEN,
                     YELLOW);
    sys_new_line(GREEN);
    printStringColor("   \\           /    \\           /      /  /|\\  \\       /       \\", GREEN, YELLOW);
    sys_new_line(GREEN);
    printStringColor("-={ see no evil }={ hear no evil }={ speak no evil }={ have no fun }=-", GREEN, YELLOW);
    sys_new_line(GREEN);
    sys_get_cursor(&printingCursor);
}

void printLogo()
{
    sys_set_cursor(&printingCursor);
    printString("    >=>>=>");
    newLine();
    printString("    >>   >=>   >>                                      >=>");
    newLine();
    printString("    >>    >=>     >====>>=>    >==>    >=>        >==> >=>         >=>");
    newLine();
    printString("    >==>>=>   >=>      >=>   >=>     >=>  >=>   >=>    >=>>=>    >=>  >=>");
    newLine();
    printString("    >>    >=> >=>    >=>    >=>     >=>    >=> >=>     >=>  >=> >=>    >=>");
    newLine();
    printString("    >>     >> >=>   >=>      >=>     >=>  >=>   >=>    >>   >=>  >=>  >=>");
    newLine();
    printString("    >===>>=>  >=> >=======>    >==>    >=>        >==> >=>  >=>    >=>");
    newLine();
    sys_get_cursor(&printingCursor);
}

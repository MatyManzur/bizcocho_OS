#include <bizcocho.h>


#define COMMAND_COUNT 14 //INCLUIDO MONKEY

int bizcochito_dummy(uint8_t argc, void** argv);

static char buffer[BUFFER_DIM]={0};

static uint8_t bufferIndex; //No es buffer circular

char* promptMessage="☻►";

static commandInfo commands[COMMAND_COUNT]={
    {.name="help", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="mem", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="ps", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="loop", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="kill", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="nice", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="block", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="cat", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="wc", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="filter", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="pipe", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="phylo", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="monke", .builtin=1, .programFunction=bizcochito_dummy },
};

void readUntilEnter()
{
    sys_write(STDOUT,promptMessage);
    do{
        sys_read(STDIN,buffer+bufferIndex,1);
        sys_write(STDOUT,buffer+bufferIndex);
    }while( buffer[bufferIndex++] !='\n'); //todo chequear buffer OVERFLORW Y ACTUAR DE FORMA ACORDE
    buffer[bufferIndex]='\0'; //Ponemos un 0 para terminar el string
}

//builtin commands help, mem, ps, kill, nice, block, sem, pipe
void bizcocho(uint8_t argc, void** argv)
{   
    sys_clear_screen();
    while (1)
    {   
        
        readUntilEnter();
        removeBackspaces(buffer);
        //Parse por pipe y despues parse por espacio
        char pipeTokens[2][MAX_PIPE_TOKEN_LENGTH]={{0}};

        int pipeTokensCount = parser(buffer, (char**)pipeTokens, '|', 2, MAX_PIPE_TOKEN_LENGTH);


        //Dependiendo de que tenemos, realizar el comando normal o llamando a la funcion builtin

    }
}

int bizcochito_dummy(uint8_t argc, void** argv)
{
    sys_write(STDOUT,"Hola soy un DUMMY PROCESS");
    return 0;
}

/* CODIGO LEGACY PARA LOGICA DE PARSEAR COMANDO, ADAPTAR Y EMBELLECER A LO NUESTRO 
        //en promptBuffer está todo lo que lee del prompt cuando se apreto enter

        char pipeTokens[2][MAX_LONG_TOKEN_LENGTH] = {{0}};

        int pipeTokensCount = parser(promptBuffer, (char**)pipeTokens, '|', 2, MAX_LONG_TOKEN_LENGTH);

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
        char firstTokens[MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {{0}};
        char secondTokens[MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {{0}};

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
            argc1 = parser(argString1, (char**)firstTokens, ' ', MAX_ARG_COUNT, MAX_TOKEN_LENGTH);
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
                argc2 = parser(argString2, (char**)secondTokens, ' ', MAX_ARG_COUNT, MAX_TOKEN_LENGTH);
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
                    sys_add_task_with_shared_screen(commands[index1].programFunction, bizcochoId, 0, argc1, (void**)argv1);
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
*/
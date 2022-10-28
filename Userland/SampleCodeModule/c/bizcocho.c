#include <bizcocho.h>
#include "tests.h"

#define COMMAND_COUNT 18 

#define NO_CHANGE_FD -2

#define IS_PIPE(c) ((c)==2)

#define GET_TOKEN_POINTER(tokens,index,length) ( ( (char *) (tokens) ) + (index) * (length) )
int8_t bizcochito_dummy(uint8_t argc, void** argv);

char* promptMessage="Bizcocho $>";

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
    {.name="phylo", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="monke", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="testsync", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_sync },
    {.name="testmm", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_mm },
    {.name="testprio", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_prio },
    {.name="testproc", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**)) test_processes },
};

void readUntilEnter(char buffer[])
{
    uint8_t bufferIndex = 0;
    do{
        if(bufferIndex == BUFFER_DIM - 1)
        {
            //Limpiamos los \b a ver si nos queda más espacio
            bufferIndex = removeBackspaces(buffer);
            if(bufferIndex == BUFFER_DIM - 1)
            {
                bufferIndex++;
                printf("\n");
                break;
            }
        }
        sys_read(STDIN, buffer + bufferIndex, 1);
        printf(buffer + bufferIndex);
    } while( buffer[bufferIndex++] !='\n');

    buffer[bufferIndex - 1]='\0'; //Ponemos un 0 para terminar el string
    removeBackspaces(buffer);
}

int8_t lookForCommandInString(char* string, uint8_t* argc, char* argv[], char** tokens){
    char *args;
    int8_t index = -1;
    for (int8_t i = 0; i < COMMAND_COUNT && index == -1; i++)
    {
        if (strPrefix(commands[i].name, string, &args))
        {
            index = i;
        }
    }

    if(index==-1){
        return index;
    }
    *argc = (uint8_t) parser(args, tokens, ' ', MAX_ARG_COUNT, MAX_TOKEN_LENGTH);
    
    for (uint8_t i = 0; i < *argc; i++)
    {
        argv[i] = GET_TOKEN_POINTER(tokens, i, MAX_TOKEN_LENGTH);
    }
    return index;
}

uint32_t executeNonBuiltIn(char* name,int8_t (*programFunction)(uint8_t argc, void** argv), uint8_t argc, void** argv, int8_t stdinChange, int8_t stdoutChange)
{   
    //TODO PIPE y DUP2
    //Cambiar fd con STDINCHANGE y STDOUTCHANGE
    if(stdinChange != NO_CHANGE_FD)
    {

    }
    if(stdoutChange != NO_CHANGE_FD)
    {
        
    }
    uint32_t pid = sys_start_child_process(name,argc,argv,programFunction);
    //Restauras los fd
    return pid;
}


//builtin commands help, mem, ps, kill, nice, block, sem, pipe
int8_t bizcocho(uint8_t argc, void** argv)
{   
    sys_clear_screen();
    while (1)
    {   
        char buffer[BUFFER_DIM]={0};
        printf(promptMessage);
        sys_set_backspace_base();
        readUntilEnter(buffer);
        //Parse por pipe y despues parse por espacio
        char pipeTokenStrings[2][MAX_PIPE_TOKEN_LENGTH]={{0}};

        uint8_t pipeTokenCount = parser(buffer, (char**)pipeTokenStrings, '|', 2, MAX_PIPE_TOKEN_LENGTH);
        
        uint8_t argc[MAX_TOKEN_COUNT];
        char* argv[MAX_TOKEN_COUNT][MAX_ARG_COUNT] = {{NULL}};
        int8_t foundCommand[MAX_TOKEN_COUNT] = {0};
        char tokens[MAX_TOKEN_COUNT][MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {{{0}}};

        uint8_t error = 0;

        if(pipeTokenCount > 2) //TODO parser nunca devuelve más que 2 porque pregunta el máximo por parámetro
        {
            fprintf(STDERR, "Cannot use pipe | more than once!\n");
            error = 1;
        }
        
        for(int k=0; k < pipeTokenCount && !error; k++)
        {
            foundCommand[k] = lookForCommandInString(pipeTokenStrings[k], &argc[k], argv[k], (char**) tokens[k]);
            // devuelve el indice del comando en el array de comandos
            // recibe: el string entero, donde tiene que dejar el int argc (int*) y el char* argv[] (char*[])
            if(foundCommand[k] < 0)
            {
                fprintf(STDERR, "Hey! That's not a valid command!\n");
                error = 1;
            }
        }
        
        if(!error)
        {
            if(IS_PIPE(pipeTokenCount))
            {
                if(commands[foundCommand[0]].builtin || commands[foundCommand[1]].builtin)
                {
                    fprintf(STDERR, "Cannot use a built-in command with a pipe!\n");
                    error = 1;
                }
                else
                {
                    //TODO pipes
                }
            }
            else
            {
                if(commands[foundCommand[0]].builtin)
                {
                    commands[foundCommand[0]].programFunction(argc[0], (void**) argv[0]);
                }
                else
                {
                    //Chequeamos si se pidio que se ejecute en background con un & al final
                    uint8_t background = (argc[0] > 0) && (strcmp(argv[0][argc[0] - 1], "&") == 0);
                    uint32_t pid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0], argv[0], background? EMPTY : NO_CHANGE_FD, NO_CHANGE_FD);
                    if(!background)
                    {
                        int8_t statusCode = sys_wait_child(pid);
                        printf("Program %s exited with status code %d !\n", commands[foundCommand[0]].name, statusCode);
                    }
                }
            }
        }
    }
}

int8_t bizcochito_dummy(uint8_t argc, void** argv)
{
    printf("HOLA\n");
    //sys_exit(6);
    return 0;
}
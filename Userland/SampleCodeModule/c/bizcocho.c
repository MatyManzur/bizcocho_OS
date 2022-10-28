#include <bizcocho.h>
#include "tests.h"
#include "testing_utils.h"
#define COMMAND_COUNT 18 

#define NO_CHANGE_FD -2

#define IS_PIPE(c) ((c)==2)

#define GET_TOKEN_POINTER(tokens,index,length) ( ( (char *) (tokens) ) + (index) * (length) )
int8_t bizcochito_dummy(uint8_t argc, void** argv);
int8_t sender(uint8_t argc, void** argv);
int8_t receiver(uint8_t argc, void** argv);

int8_t kill(uint8_t argc, void* argv[]);
int8_t block(uint8_t argc, void* argv[]);
int8_t nice(uint8_t argc, void* argv[]);

char* promptMessage="Bizcocho $>";

static commandInfo commands[COMMAND_COUNT]={
    {.name="help", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="mem", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="ps", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="loop", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="kill", .builtin=1, .programFunction=kill },
    {.name="nice", .builtin=1, .programFunction=nice },
    {.name="block", .builtin=1, .programFunction=block },
    {.name="cat", .builtin=0, .programFunction=sender },
    {.name="wc", .builtin=0, .programFunction=receiver },
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
    if(stdinChange != NO_CHANGE_FD)
    {
        if(sys_dup2(stdinChange, STDIN) != 0)
        {
            fprintf(STDERR, "Error in replacing fileDescriptors!\n");
            return 0;
        }
    }
    if(stdoutChange != NO_CHANGE_FD)
    {
        if(sys_dup2(stdoutChange, STDOUT) != 0)
        {
            fprintf(STDERR, "Error in replacing fileDescriptors!\n");
            return 0;
        }
    }
    uint32_t pid = sys_start_child_process(name,argc,argv,programFunction);
    sys_revert_fd_replacements();
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
                    continue;
                }

                if(sys_mkpipe("pipe")!=0)
                {
                    fprintf(STDERR, "Error in creating pipe!\n");
                    continue;
                }

                uint8_t writeFd;
                if(sys_open("pipe", 'W', &writeFd) != 0)
                {
                    fprintf(STDERR, "Error in opening writing end of pipe!\n");
                    continue;
                }
                uint32_t leftPid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0], argv[0], NO_CHANGE_FD, writeFd);
                sys_close(writeFd);
                if(leftPid == 0)
                    continue;

                uint8_t readFd;
                if(sys_open("pipe", 'R', &readFd) != 0)
                {
                    fprintf(STDERR, "Error in opening reading end of pipe!\n");
                    continue;
                }
                uint32_t rightPid = executeNonBuiltIn(commands[foundCommand[1]].name, commands[foundCommand[1]].programFunction, argc[1], argv[1], readFd, NO_CHANGE_FD);
                sys_close(readFd);
                if(rightPid == 0)
                    continue;
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
                    uint32_t pid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0] - background, argv[0], background? EMPTY : NO_CHANGE_FD, NO_CHANGE_FD);
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

int8_t sender(uint8_t argc, void** argv)
{
    printf("HOLA\n");
    sys_exit(0);
    return 0;
}

int8_t receiver(uint8_t argc, void** argv)
{
    char c = 1;
    while(c!=0)
    {
        sys_read(STDIN, &c, 1);
        printf("Recibido: %c\n", c);
    }
    sys_exit(0);
    return 0;
}
int8_t kill(uint8_t argc, void* argv[]){
    if(argc != 1){
        fprintf(STDERR,"Error! Invalida cantidad de argumentos, Recibidos: %d y Necesarios: 1",argc);
        return -1;
    }
    uint32_t pid= satoi((char*)argv[0]);
    if(!sys_kill_process(pid)){
        fprintf(STDERR,"Error! No se encontro el proceso con PID: %d",pid);
        return -1;
    }else{
        printf("PID:%d has been SLAIN",pid);
    }
    return 0;
}
int8_t block(uint8_t argc, void* argv[]){
    if(argc != 1){
        fprintf(STDERR,"Error! Invalida cantidad de argumentos, Recibidos: %d y Necesarios: %d",argc,1);
        return -1;
    }
    uint32_t pid= satoi((char*)argv[0]);
    if(!sys_block_process(pid)){
        fprintf(STDERR,"Error! No se encontro el proceso con PID: %d",pid);
        return -1;
    }
    return 0;
}
int8_t nice(uint8_t argc, void* argv[]){
    if(argc != 2){
        fprintf(STDERR,"Error! Invalida cantidad de argumentos, Recibidos: %d y Necesarios: %d",argc,2);
        return -1;
    }
    uint32_t pid = satoi((char *) argv[0]);
    uint8_t priority = satoi((char *) argv[1]);
    if(!sys_change_priority(pid,priority)){
        fprintf(STDERR,"Error! No se encontro el proceso con PID: %d O la prioridad dada es invalida deberia ser [0-4]",pid);
        return -1;
    }
    return 0;
}
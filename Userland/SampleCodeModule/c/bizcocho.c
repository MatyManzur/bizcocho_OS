// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <bizcocho.h>
#include "tests.h"

#define COMMAND_COUNT 20

#define NO_CHANGE_FD -2

#define IS_PIPE(c) ((c)==2)



char* promptMessage="Bizcocho $>";

format_t promptColor= {.backgroundColor=DEFAULT, .characterColor=L_RED};

static uint32_t bizcochoPid = 0;
static uint32_t pipesCreated = 1;
static commandInfo commands[COMMAND_COUNT]={
    {.name="help", .builtin=1, .programFunction=help },
    {.name="mem", .builtin=1, .programFunction=mem },
    {.name="ps", .builtin=1, .programFunction=ps },
    {.name="loop", .builtin=0, .programFunction=loop },
    {.name="kill", .builtin=1, .programFunction=kill },
    {.name="nice", .builtin=1, .programFunction=nice },
    {.name="block", .builtin=1, .programFunction=block },
    {.name="cat", .builtin=0, .programFunction=cat },
    {.name="wc", .builtin=0, .programFunction=wc },
    {.name="filter", .builtin=0, .programFunction=filter },
    {.name="pipe", .builtin=1, .programFunction=pipe },
    {.name="sem", .builtin=1, .programFunction=sem },
    {.name="phylo", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**)) startPhylo },
    {.name="monke", .builtin=1, .programFunction=monke },
    {.name="color", .builtin=1, .programFunction=color },
    {.name="testsync", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_sync },
    {.name="testmm", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_mm },
    {.name="testprio", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_prio },
    {.name="testproc", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**)) test_processes },
    {.name="clear", .builtin=1, .programFunction=(int8_t (*)(uint8_t, void**)) sys_clear_screen},
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
        printf("%s", buffer + bufferIndex);
    } while( buffer[bufferIndex++] !='\n');

    buffer[bufferIndex - 1]='\0'; //Ponemos un 0 para terminar el string
    removeBackspaces(buffer);
}

int8_t lookForCommandInString(char* string, uint8_t* argc, char* argv[], char tokens[][MAX_TOKEN_LENGTH]){
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
    *argc = (uint8_t) parser(args, tokens, ' ', MAX_ARG_COUNT);
    
    for (uint8_t i = 0; i < *argc; i++)
    {
        argv[i] = tokens[i];
    }
    return index;
}

uint32_t executeNonBuiltIn(char* name,int8_t (*programFunction)(uint8_t argc, void** argv), uint8_t argc, void** argv, int8_t stdinChange, int8_t stdoutChange, uint8_t background)
{   
    if(stdinChange != NO_CHANGE_FD)
    {
        if(sys_dup2(stdinChange, STDIN) != 0)
        {
            fprintf(STDERR, "Error in replacing fileDescriptors!\n");
            return 0;
        }
        sys_close(stdinChange);
    }
    if(stdoutChange != NO_CHANGE_FD)
    {
        if(sys_dup2(stdoutChange, STDOUT) != 0)
        {
            fprintf(STDERR, "Error in replacing fileDescriptors!\n");
            return 0;
        }
        sys_close(stdoutChange);
    }
    uint32_t pid;
    if(background)
    {
        pid = sys_start_parent_process(name, argc, argv, programFunction, 2 ,bizcochoPid); //Hacemos que copie los FDs de Bizcocho
    }
    else
    {
        pid = sys_start_child_process(name,argc,argv,programFunction, 1);
        sys_change_priority(pid, 2);
    }
    sys_revert_fd_replacements();
    return pid;
}


int8_t bizcocho(uint8_t argc, void** argv)
{   
    sys_clear_screen();
    bizcochoPid = sys_get_pid();
    while (1)
    {   
        char buffer[BUFFER_DIM]={0};
        sys_print_to_stdout_color(promptMessage, promptColor);
        sys_clean_buffer();
        sys_set_backspace_base();
        readUntilEnter(buffer);
        //Parsea por pipe y despues parse por espacio
        char pipeTokenStrings[2][MAX_TOKEN_LENGTH]={{0}};

        uint8_t pipeTokenCount = parser(buffer, pipeTokenStrings, '|', 2);
        
        uint8_t argc[MAX_TOKEN_COUNT];
        char* argv[MAX_TOKEN_COUNT][MAX_ARG_COUNT] = {{NULL}};
        int8_t foundCommand[MAX_TOKEN_COUNT] = {0};
        char tokens[MAX_TOKEN_COUNT][MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {{{0}}};

        uint8_t error = 0;

        if(pipeTokenCount == 0) //si apreta solo enter sin escribir nada
            continue;
        
        for(int k=0; k < pipeTokenCount && !error; k++)
        {
            foundCommand[k] = lookForCommandInString(pipeTokenStrings[k], &argc[k], argv[k],tokens[k]);
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
                char namepipe[MAX_PIPE_NAME_SIZE] = {0};
                snprintf(namepipe,MAX_PIPE_NAME_SIZE,"Pipe %d",pipesCreated);
                if(sys_mkpipe(namepipe)!=0)
                {
                    fprintf(STDERR, "Error in creating pipe!\n");
                    continue;
                }
                pipesCreated++;
                uint8_t writeFd;
                if(sys_open(namepipe, 'W', &writeFd) != 0)
                {
                    fprintf(STDERR, "Error in opening writing end of pipe!\n");
                    continue;
                }

                uint8_t leftBackground = (argc[0] > 0) && (strcmp(argv[0][argc[0] - 1], "&") == 0);
                uint32_t leftPid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0] - leftBackground,(void**)argv[0], NO_CHANGE_FD, writeFd, leftBackground);
                if(leftPid == 0)
                    continue;
                
                uint8_t readFd;
                if(sys_open(namepipe, 'R', &readFd) != 0)
                {
                    fprintf(STDERR, "Error in opening reading end of pipe!\n");
                    sys_kill_process(leftPid);
                    continue;
                }

                uint8_t rightBackground = (argc[1] > 0) && (strcmp(argv[1][argc[1] - 1], "&") == 0);
                uint32_t rightPid = executeNonBuiltIn(commands[foundCommand[1]].name, commands[foundCommand[1]].programFunction, argc[1] - rightBackground,(void**) argv[1], readFd, NO_CHANGE_FD, rightBackground);
                if(rightPid == 0)
                {
                    sys_kill_process(leftPid);
                    continue;
                }
                if(!leftBackground)
                {
                    int8_t statusCode = sys_wait_child(leftPid);
                    printf("Program %s exited with status code %d !\n", commands[foundCommand[0]].name, statusCode);
                }
                if(!rightBackground)
                {
                    int8_t statusCode = sys_wait_child(rightPid);
                    printf("Program %s exited with status code %d !\n", commands[foundCommand[1]].name, statusCode);
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
                    uint32_t pid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0] - background,(void**)argv[0], background? EMPTY : NO_CHANGE_FD, NO_CHANGE_FD, background);
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
int8_t kill(uint8_t argc, void* argv[])
{
    CHECK_ARGC(argc,1)
    uint32_t pid= satoi((char*)argv[0]);
    if(pid==bizcochoPid)
    {
        fprintf(STDERR, "HA! You thought I'd let you beat me so easily?!\n");
        return -1;
    }
    if(!sys_kill_process(pid))
    {
        if(pid==1)
            fprintf(STDERR, "Don't mess with poor init! He's just doing his job\n");
        else
            fprintf(STDERR,"Error! Couldn't find process with PID: %d\n",pid);
        return -1;
    }
    else
    {
        printf("PID:%d has been SLAIN\n",pid);
    }
    return 0;
}

int8_t block(uint8_t argc, void* argv[])
{
    CHECK_ARGC(argc,1)
    uint32_t pid= satoi((char*)argv[0]);
    if(pid==bizcochoPid)
    {
        fprintf(STDERR, "Nah, I don't feel much like sleeping now :)\n");
        return -1;
    }
    if(!sys_block_process(pid))
    {
        if(pid==1)
            fprintf(STDERR, "Don't mess with poor init! He's just doing his job\n");
        else
            fprintf(STDERR, "Error! Couldn't find process with PID: %d\n",pid);
        return -1;
    }
    printf("Process of PID %d has been blocked\n", pid);
    return 0;
}

//Se le pasa un string, un buffer donde dejara los tokens, el char separador de tokens, una cantidad maxima de tokens y la longitud maxima de cada token. 
//La funcion parsea con el char provisto el string en tokens, si se llega a la longitud maxima en un token el mismo quedara con esa longitud y si se llega a la cantidad maxima de tokens se dejara de parsear, 
//si esta ultima no se alcanza entonces parsea hasta el final del string. Devuelve por parametro la cantidad de tokens que llego a parsear.
int parser(char *string, char buffer[][MAX_TOKEN_LENGTH], char separator, int maxTokenCount)
{
    if (maxTokenCount == 0)
    {
        return -1;
    }
    int count = 0;
    int j = 0;
    for (int i = 0; string[i] != '\0' && (count != maxTokenCount); i++)
    {
        if (string[i] == separator || j == MAX_TOKEN_LENGTH)
        {
            if (j != 0)
            {
                buffer[count++][j] = '\0';
                j = 0;
            }
        } else
        {
            buffer[count][j++] = string[i];
        }
    }
    if (j != 0)
    {
        buffer[count++][j] = '\0';
    }
    return count;
}
#include <bizcocho.h>
#include "tests.h"
#include "testing_utils.h"
#define COMMAND_COUNT 18
#define CHECK_ARGC(argc,amount){\
    if(argc != amount){\
        fprintf(STDERR, "Error! Invalid argument count, Received %d when %d was necessary\n",argc,amount);\
        return -1;}};
#define NO_CHANGE_FD -2

#define IS_PIPE(c) ((c)==2)

#define IS_VOWEL(c) ((c)=='A' || (c)=='E' || (c)=='I' || (c)=='O' || (c)=='U' || (c)=='a' || (c)=='e' || (c)=='i' || (c)=='o' || (c)=='u')

#define GET_TOKEN_POINTER(tokens,index,length) ( ( (char *) (tokens) ) + (index) * (length) )
int8_t bizcochito_dummy(uint8_t argc, void** argv);
int8_t hitman(uint8_t argc, void** argv);
int8_t cat(uint8_t argc, void** argv);
int8_t wc(uint8_t argc, void** argv);
int8_t filter(uint8_t argc, void** argv);

int8_t kill(uint8_t argc, void* argv[]);
int8_t block(uint8_t argc, void* argv[]);
int8_t nice(uint8_t argc, void* argv[]);
int8_t ps(uint8_t argc, void* argv[]);
int8_t mem(uint8_t argc, void* argv[]);
int8_t loop(uint8_t argc, void* argv[]);
char* promptMessage="Bizcocho $>";

static uint32_t bizcochoPid = 0;
static size_t pipesCreated = 1;
static commandInfo commands[COMMAND_COUNT]={
    {.name="help", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="mem", .builtin=1, .programFunction=mem },
    {.name="ps", .builtin=1, .programFunction=ps },
    {.name="loop", .builtin=0, .programFunction=loop },
    {.name="kill", .builtin=1, .programFunction=kill },
    {.name="nice", .builtin=1, .programFunction=nice },
    {.name="block", .builtin=1, .programFunction=block },
    {.name="cat", .builtin=0, .programFunction=cat },
    {.name="wc", .builtin=0, .programFunction=wc },
    {.name="filter", .builtin=0, .programFunction=filter },
    {.name="pipe", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="phylo", .builtin=0, .programFunction=bizcochito_dummy },
    {.name="monke", .builtin=1, .programFunction=bizcochito_dummy },
    {.name="testsync", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_sync },
    {.name="testmm", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_mm },
    {.name="testprio", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**))test_prio },
    {.name="testproc", .builtin=0, .programFunction=(int8_t (*)(uint8_t, void**)) test_processes },
    {.name="clear", .builtin=1, .programFunction=sys_clear_screen},
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

uint32_t executeNonBuiltIn(char* name,int8_t (*programFunction)(uint8_t argc, void** argv), uint8_t argc, void** argv, int8_t stdinChange, int8_t stdoutChange, uint8_t background)
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
    uint32_t pid;
    if(background)
    {
        pid = sys_start_parent_process(name, argc, argv, programFunction, 2 ,bizcochoPid); //Hacemos que copie los FDs de Bizcocho
    }
    else
    {
        pid = sys_start_child_process(name,argc,argv,programFunction);
        sys_change_priority(pid, 2);
    }
    
    sys_revert_fd_replacements();
    return pid;
}


//builtin commands help, mem, ps, kill, nice, block, sem, pipe (TODO: borrar)
int8_t bizcocho(uint8_t argc, void** argv)
{   
    sys_clear_screen();
    bizcochoPid = sys_get_pid();
    while (1)
    {   
        char buffer[BUFFER_DIM]={0};
        printf(promptMessage);
        sys_clean_buffer();
        sys_set_backspace_base();
        readUntilEnter(buffer);
        //Parsea por pipe y despues parse por espacio
        char pipeTokenStrings[2][MAX_PIPE_TOKEN_LENGTH]={{0}};

        uint8_t pipeTokenCount = parser(buffer, (char**)pipeTokenStrings, '|', 2, MAX_PIPE_TOKEN_LENGTH);
        
        uint8_t argc[MAX_TOKEN_COUNT];
        char* argv[MAX_TOKEN_COUNT][MAX_ARG_COUNT] = {{NULL}};
        int8_t foundCommand[MAX_TOKEN_COUNT] = {0};
        char tokens[MAX_TOKEN_COUNT][MAX_ARG_COUNT][MAX_TOKEN_LENGTH] = {{{0}}};

        uint8_t error = 0;

        if(pipeTokenCount == 0) //si apreta solo enter sin escribir nada
            continue;
        
        for(int k=0; k < pipeTokenCount && !error; k++)
        {
            foundCommand[k] = lookForCommandInString(pipeTokenStrings[k], &argc[k], argv[k], (char**) tokens[k]);
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
                uint8_t readFd;
                if(sys_open(namepipe, 'R', &readFd) != 0)
                {
                    fprintf(STDERR, "Error in opening reading end of pipe!\n");
                    sys_close(writeFd);
                    continue;
                }

                uint8_t leftBackground = (argc[0] > 0) && (strcmp(argv[0][argc[0] - 1], "&") == 0);
                uint32_t leftPid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0] - leftBackground, argv[0], NO_CHANGE_FD, writeFd, leftBackground);
                sys_close(writeFd);
                if(leftPid == 0)
                    continue;
                
                uint8_t rightBackground = (argc[1] > 0) && (strcmp(argv[1][argc[1] - 1], "&") == 0);
                uint32_t rightPid = executeNonBuiltIn(commands[foundCommand[1]].name, commands[foundCommand[1]].programFunction, argc[1] - rightBackground, argv[1], readFd, NO_CHANGE_FD, rightBackground);
                sys_close(readFd);
                if(rightPid == 0)
                {
                    sys_kill_process(leftPid);
                    continue;
                }
                uint8_t hitmanArgc=0;
                uint32_t* hitmanArgv[2];
                if(!leftBackground)
                    hitmanArgv[hitmanArgc++] = leftPid;
                if(!rightBackground)
                    hitmanArgv[hitmanArgc++] = rightPid;
                uint32_t hitmanPid = 0;
                if(hitmanArgc > 0)
                    hitmanPid = sys_start_child_process("hitman", hitmanArgc, hitmanArgv, hitman);
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
                if(hitmanPid != 0)
                {
                    sys_kill_process(hitmanPid);
                    sys_wait_child(hitmanPid);
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
                    uint32_t pid = executeNonBuiltIn(commands[foundCommand[0]].name, commands[foundCommand[0]].programFunction, argc[0] - background, argv[0], background? EMPTY : NO_CHANGE_FD, NO_CHANGE_FD, background);
                    uint32_t hitmanPid = 0;
                    if(!background)
                    {
                        uint32_t* hitmanArgv[1] = {pid};
                        hitmanPid = sys_start_child_process("hitman", 1, hitmanArgv, hitman);
                    }
                    if(!background)
                    {
                        int8_t statusCode = sys_wait_child(pid);
                        printf("Program %s exited with status code %d !\n", commands[foundCommand[0]].name, statusCode);
                    }
                    if(hitmanPid != 0)
                    {
                        sys_kill_process(hitmanPid);
                        sys_wait_child(hitmanPid);
                    }
                }
            }
        }
    }
}

int8_t hitman(uint8_t argc, void** argv)
{
    if(argc>0)
    {
        uint32_t** targetPids = (uint32_t**)argv;
        char c = 0;
        while(c!='q')
        {
            sys_read(STDIN, &c, 1);
        }
        for(int i=0; i<argc; i++)
        {
            sys_kill_process(targetPids[i]);
        }
    }
    sys_exit(0);
    return 0;
}

int8_t bizcochito_dummy(uint8_t argc, void** argv)
{
    printf("HOLA\n");
    return 0;
}

int8_t cat(uint8_t argc, void** argv)
{
    char c = 1;
    while(c!=0)
    {
        sys_read(STDIN, &c, 1);
        printf("%c", c);
    }
    sys_exit(0);
    return 0;
}

int8_t wc(uint8_t argc, void** argv)
{
    char c = 1;
    uint16_t lines = 0;
    while(c!=0)
    {
        sys_read(STDIN, &c, 1);
        if(c=='\n')
            lines++;
    }
    printf("Total lines count: %d\n", lines);
    sys_exit(0);
    return 0;
}

int8_t filter(uint8_t argc, void** argv)
{
    char c = 1;
    while(c!=0)
    {
        sys_read(STDIN, &c, 1);
        if(!IS_VOWEL(c))
        {
            printf("%c", c);
        }
    }
    sys_exit(0);
    return 0;
}

int8_t kill(uint8_t argc, void* argv[])
{
    CHECK_ARGC(argc,1)
    uint32_t pid= satoi((char*)argv[0]);
    if(pid==bizcochoPid)
    {
        fprintf(STDERR, "HA! You think I'd let you beat me so easily?!\n");
        return -1;
    }
    if(!sys_kill_process(pid))
    {
        fprintf(STDERR,"Error! Couldn't find process with PID: %d\n",pid);
        return -1;
    }
    else
    {
        printf("PID:%d has been SLAIN\n",pid);
    }
    return 0;
}

int8_t ps(uint8_t argc, void* argv[])
{
    printProcessesTable();
    return 0;
}

int8_t block(uint8_t argc, void* argv[]){
    CHECK_ARGC(argc,1)
    uint32_t pid= satoi((char*)argv[0]);
    if(pid==bizcochoPid)
    {
        fprintf(STDERR, "Nah, I don't feel much like sleeping now :)\n");
        return -1;
    }
    if(!sys_block_process(pid))
    {
        fprintf(STDERR,"Error! Couldn't find process with PID: %d\n",pid);
        return -1;
    }
    return 0;
}

int8_t nice(uint8_t argc, void* argv[])
{
    CHECK_ARGC(argc,2)
    uint32_t pid = satoi((char *) argv[0]);
    uint8_t priority = satoi((char *) argv[1]);
    if(!sys_change_priority(pid,priority))
    {
        fprintf(STDERR,"Error! Couldn't find process with PID: %d or given priority value is invalid: should be [0-4]\n",pid);
        return -1;
    }
    return 0;
}
int8_t mem(uint8_t argc, void* argv[]){
    printMemState();
    return 0;
}
int8_t loop(uint8_t argc, void* argv[]){
    uint32_t pid=sys_get_pid();
    struct datetime_t previousTime;
    struct datetime_t timeAtCheck={0};
    struct timezone_t timezone;
    sys_get_current_date_time(&timeAtCheck,&timezone);
    previousTime.secs=timeAtCheck.secs;
    while(1){
        sys_get_current_date_time(&timeAtCheck,&timezone);
        if( ( previousTime.secs+5 )<timeAtCheck.secs || previousTime.secs > timeAtCheck.secs)
        {
            previousTime.secs=timeAtCheck.secs;
            printf("Hola soy el Loop con PID:%d \n",pid);
        }
        sys_yield();
    }
}
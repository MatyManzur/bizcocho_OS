#include <commands.h>
#define IS_VOWEL(c) ((c)=='A' || (c)=='E' || (c)=='I' || (c)=='O' || (c)=='U' || (c)=='a' || (c)=='e' || (c)=='i' || (c)=='o' || (c)=='u')

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



int8_t ps(uint8_t argc, void* argv[])
{
    printProcessesTable();
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
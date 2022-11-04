// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <commands.h>
#define IS_VOWEL(c) ((c) == 'A' || (c) == 'E' || (c) == 'I' || (c) == 'O' || (c) == 'U' || (c) == 'a' || (c) == 'e' || (c) == 'i' || (c) == 'o' || (c) == 'u')

int8_t help(uint8_t argc, void **argv)
{
    format_t highlightColor = {.backgroundColor = DEFAULT, .characterColor = PINK};
    if (argc >= 1)
    {
        if (!strcmp((char *)argv[0], "1"))
        {
            argc = 0;
        }
        else if (!strcmp((char *)argv[0], "2"))
        {
            for (int i = 0; i < 80; i++)
                printf("-");
            printf("  PROCESS commands:  \n");
            for (int i = 0; i < 80; i++)
                printf("-");
            sys_print_to_stdout_color("NOTE:", highlightColor);
            printf(" With all of these you can use '<name> &' to run them in background,\n");
            printf("or '<name1> | <name2>' to connect the output of the first with input of the second with a pipe, ");
            printf("or combine both as in '<name1> & | <name2> &' \n");
            sys_print_to_stdout_color("NOTE 2:", highlightColor);
            printf(" If the process is not sent to background, Bizcocho will block until the process is finished or KILLED with [ESC] key\n");
            sys_print_to_stdout_color("NOTE 3:", highlightColor);
            printf(" To send an EOF to STDIN press [Ctrl + D]\n");
            for (int i = 0; i < 80; i++)
                printf("-");
            printf("%c 'loop': sends a message every 5 seconds \n", 7);
            printf("%c 'phylo': philosophers program \n", 7);
            printf("%c 'testmm <x>': memory allocation test. <x> is the max bytes that it will use \n", 7);
            printf("%c 'testprio': priority changing processes test\n", 7);
            printf("%c 'testproc <x>': process creation test. <x> is the max count of processes\n", 7);
            printf("%c 'testsync <x> <y>': semaphore test. <x> is how many inc/decrements each process does, <y>=1 to use semaphores and <y>=0 to not use them\n", 7);
            printf("%c 'cat': prints to output whatever reads from its input until an EOF is found\n", 7);
            printf("%c 'wc': reads from input until an EOF, and then prints the line count of what has read\n", 7);
            printf("%c 'filter': same as cat, but filters the vowels\n", 7);
            for (int i = 0; i < 80; i++)
                printf("-");
            return 0;
        }
        else if (!strcmp((char *)argv[0], "color"))
        {
            for (int i = 0; i < 80; i++)
                printf("-");
            printf("  Available COLORS:  \n");
            for (int i = 0; i < 80; i++)
                printf("-");
            char *colorNames[WHITE + 1] = {"BLACK", "BLUE", "GREEN", "CYAN", "RED", "MAGENTA", "BROWN", "LIGHT GRAY",
                                           "DARK GRAY", "LIGHT BLUE", "LIGHT GREEN", "LIGHT CYAN", "LIGHT RED", "PINK", "YELLOW", "WHITE"};
            format_t fmt = {.characterColor = WHITE};
            for (fmt.backgroundColor = BLACK; fmt.backgroundColor <= WHITE; fmt.backgroundColor++)
            {
                printf("%d : ", fmt.backgroundColor);
                if (fmt.backgroundColor == L_GREEN)
                    fmt.characterColor = BLACK;
                sys_print_to_stdout_color(colorNames[fmt.backgroundColor], fmt);
                printf("\n");
            }
            return 0;
        }
        else
        {
            printf("Help page not found\n");
            return 0;
        }
    }
    else if (argc == 0)
    {
        for (int i = 0; i < 40; i++)
        {
            printf("%c", (i % 2) + 1);
            printf("-");
        }
        printf("  Welcome to Bizcocho! Available commands: \n");
        for (int i = 0; i < 40; i++)
        {
            printf("%c", (i % 2) + 1);
            printf("-");
        }
        printf("  See '");
        sys_print_to_stdout_color("help 2", highlightColor);
        printf("' for Process commands in following help page!! \n");
        printf("  List of BUILT-IN commands (not processes): \n");
        for (int i = 0; i < 80; i++)
            printf("-");
        printf("%c 'help': what you're seeing now \n", 7);
        printf("%c 'mem': see memory allocation system state \n", 7);
        printf("%c 'ps': detailed list of current processes \n", 7);
        printf("%c 'pipe': detailed list of currently open pipes \n", 7);
        printf("%c 'sem': detailed list of currently open semaphores \n", 7);
        printf("%c 'kill <x>': kills process with pid <x> \n", 7);
        printf("%c 'nice <x> <y>': changes priority value of process with pid <x> to a value of <y>. Priority value must be in [0-4]\n", 7);
        printf("%c 'block <x>': blocks process with pid <x> \n", 7);
        printf("%c 'color <x> <y>': changes the console's colors to <x> for background and <y> for characters colors \n", 7);
        printf("%c 'help color': see list of available colors \n", 7);
        printf("%c 'monke': prints some funny monkeys! \n", 7);
        printf("%c 'clear': clears the console \n", 7);
        for (int i = 0; i < 80; i++)
            printf("-");
    }
    return 0;
}

int8_t monke(uint8_t argc, void **argv)
{
    format_t fmt = {.backgroundColor = GREEN, .characterColor = YELLOW};
    sys_print_to_stdout_color("       .-\"-.            .-\"-.            .-\"-.           .-\"-.                 \n", fmt);
    sys_print_to_stdout_color("     _/_-.-_\\_        _/.-.-.\\_        _/.-.-.\\_       _/.-.-.\\_               \n", fmt);
    sys_print_to_stdout_color("    / __} {__ \\      /|( o o )|\\      ( ( o o ) )     ( ( o o ) )              \n", fmt);
    sys_print_to_stdout_color("   / //  \"  \\ \\     | //  \"  \\ |       |/  \"  \\|       |/  \"  \\|               \n", fmt);
    sys_print_to_stdout_color("  / / \\'---'/ \\ \\  / / \\'---'/ \\ \\      \\'/^\\'/         \\ .-. /                \n", fmt);
    sys_print_to_stdout_color("  \\ \\_/`\"\"\"`\\_/ /  \\ \\_/`\"\"\"`\\_/ /      /`\\ /`\\         /`\"\"\"`\\                \n", fmt);
    sys_print_to_stdout_color("   \\           /    \\           /      /  /|\\  \\       /       \\               \n", fmt);
    sys_print_to_stdout_color("-={ see no evil }={ hear no evil }={ speak no evil }={ have no fun }=-         \n", fmt);
    return 0;
}

int8_t cat(uint8_t argc, void **argv)
{
    char c = 1;
    while (c != 0)
    {
        sys_read(STDIN, &c, 1);
        printf("%c", c);
    }
    sys_exit(0);
    return 0;
}

int8_t wc(uint8_t argc, void **argv)
{
    char c = 1;
    uint16_t lines = 0;
    while (c != 0)
    {
        sys_read(STDIN, &c, 1);
        if (c == '\n')
            lines++;
    }
    printf("Total lines count: %d\n", lines);
    sys_exit(0);
    return 0;
}

int8_t filter(uint8_t argc, void **argv)
{
    char c = 1;
    while (c != 0)
    {
        sys_read(STDIN, &c, 1);
        if (!IS_VOWEL(c))
        {
            printf("%c", c);
        }
    }
    sys_exit(0);
    return 0;
}

int8_t color(uint8_t argc, void **argv)
{
    CHECK_ARGC(argc, 2)
    color_t backgroundColor = satoi((char *)argv[0]);
    color_t characterColor = satoi((char *)argv[1]);
    if (backgroundColor < BLACK || backgroundColor > WHITE || characterColor < BLACK || characterColor > WHITE)
    {
        fprintf(STDERR, "Invalid color. Use 'help color' to see available colors!\n");
        return 0;
    }
    sys_change_color(backgroundColor, characterColor);
    return 0;
}

int8_t ps(uint8_t argc, void **argv)
{
    printProcessesTable();
    return 0;
}

int8_t sem(uint8_t argc, void **argv)
{
    printSemaphoreTable();
    return 0;
}

int8_t nice(uint8_t argc, void **argv)
{
    CHECK_ARGC(argc, 2)
    uint32_t pid = satoi((char *)argv[0]);
    uint8_t priority = satoi((char *)argv[1]);
    if (!sys_change_priority(pid, priority))
    {
        fprintf(STDERR, "Error! Couldn't find process with PID: %d or given priority value is invalid: should be [0-4]\n", pid);
        return -1;
    }
    printf("Priority of process with PID %d has been changed to %d\n", pid, priority);
    return 0;
}
int8_t mem(uint8_t argc, void **argv)
{
    printMemInfo();
    return 0;
}
int8_t pipe(uint8_t argc, void **argv)
{
    printPipeTable();
    return 0;
}
int8_t loop(uint8_t argc, void **argv)
{
    uint32_t pid = sys_get_pid();
    struct datetime_t previousTime;
    struct datetime_t timeAtCheck = {0};
    struct timezone_t timezone = {.hours = -3, .minutes = 0};
    sys_set_time_zone(&timezone);
    sys_get_current_date_time(&timeAtCheck, &timezone);
    previousTime.secs = timeAtCheck.secs;
    while (1)
    {
        sys_get_current_date_time(&timeAtCheck, &timezone);
        if ((previousTime.secs + 5) <= timeAtCheck.secs || previousTime.secs > timeAtCheck.secs)
        {
            previousTime.secs = timeAtCheck.secs;
            printf("Hi! I'm loop with PID:%d. It's: %2d:%2d:%2d - %2d/%2d/%4d  UTC%d\n", pid, timeAtCheck.hours, timeAtCheck.mins, timeAtCheck.secs,
                   timeAtCheck.day, timeAtCheck.month, timeAtCheck.year, timezone.hours);
        }
        sys_yield();
    }
}
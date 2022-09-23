#include <programRunner.h>

#define HEIGHT 25
#define WIDTH 80

void runner(uint8_t argc, void **argv) //argv[3 ó 6] = {(functionPointer_t*)firstFuncPointer, (uint8_t*)firstargc, (void**)firstargv, (functionPointer_t*)secondFuncPointer, (uint8_t*)secondargc, (void**)secondargv}
{
    uint8_t firstTaskId, secondTaskId;
    uint8_t firstTaskPaused = 0, secondTaskPaused = 0;
    uint8_t firstTaskKilled = 0, secondTaskKilled = 0;


    if (argc == 3) //Me pidieron uno solo, printea en el bizocho
    {
        if (((functionPointer_t *) argv[0])->function == NULL)  //si no pasaron el puntero a funcion, termina
            sys_exit();

        int runnerId = sys_get_task_id(); //este task tiene el mismo screenId que bizcocho, entonces pedir el taskId de runner es lo mismo.

        //agregamos la task pasandole runnerId -> va a compartir screen con runner -> comparte screen con bizcocho
        firstTaskId = sys_add_task_with_shared_screen(((functionPointer_t *) argv[0])->function, runnerId, 0,
                                                      *((uint8_t *) argv[1]), (void **) argv[2]);

    } else if (argc == 6) //Me pidieron dos, hacemos el pipe
    {
        sys_clear_screen(BLACK);    //limpiamos la pantalla
        point_t divisionPoint = {0, (WIDTH / 2) - 1};    //dibujamos la linea separadora del medio
        for (; divisionPoint.row < HEIGHT; divisionPoint.row++)
        {
            divisionPoint.column = (WIDTH / 2) - 1;
            sys_set_cursor(&divisionPoint);
            putCharColor(219, BLACK, WHITE);    //lo hacemos de ancho 2, asi quedan 39 para cada programa
            putCharColor(219, BLACK, WHITE);
        }

        if (((functionPointer_t *) argv[0])->function == NULL) //si no nos pasaron los punteros a funcion, termina
            sys_exit();
        if (((functionPointer_t *) argv[3])->function == NULL)
            sys_exit();

        //agregamos la task en la mitad izquierda
        point_t topLeft = {0, 0};
        point_t bottomRight = {HEIGHT - 1, (WIDTH / 2) - 2};

        firstTaskId = sys_add_task(((functionPointer_t *) argv[0])->function, &topLeft, &bottomRight, 0,
                                   *((uint8_t *) argv[1]), (void **) argv[2]);

        //agregamos la task en la mitad derecha
        topLeft = (point_t) {0, (WIDTH / 2) + 1};
        bottomRight = (point_t) {HEIGHT - 1, WIDTH - 1};

        secondTaskId = sys_add_task(((functionPointer_t *) argv[3])->function, &topLeft, &bottomRight, 0,
                                    *((uint8_t *) argv[4]), (void **) argv[5]);

    } else //no me pasaron ni 3 ni 6 args
    {
        sys_exit();
    }
    struct kbEvent_t key = {0};    //el runner se queda fijandose si se apreto una tecla
    while (key.key != VK_ESCAPE || key.action != RELEASED) //si fue un ESCAPE, termina
    {
        sys_get_next_key(&key);    //se fija la tecla
        if (key.key == VK_N && key.action == PRESSED)    //si es una N pausamos o reanudamos la firstTask
        {
            if (firstTaskPaused)
            {
                firstTaskPaused = 0;
                sys_activate_task(firstTaskId);
            } else
            {
                firstTaskPaused = 1;
                sys_deactivate_task(firstTaskId);
            }
        }
        if (argc == 6) //si hay pipe
        {
            if (key.key == VK_M && key.action == PRESSED) //si es una M pausamos o reanudamos la secondTask
            {
                if (secondTaskPaused)
                {
                    secondTaskPaused = 0;
                    sys_activate_task(secondTaskId);
                } else
                {
                    secondTaskPaused = 1;
                    sys_deactivate_task(secondTaskId);
                }
            }
            if (!firstTaskKilled && key.key == VK_J && key.action == PRESSED) //si se apreta la J se mata la firstTask
            {
                firstTaskKilled = 1;
                sys_kill_task(firstTaskId);
            }
            if (!secondTaskKilled && key.key == VK_K && key.action == PRESSED) //si se apreta la K se mata la secondTask
            {
                secondTaskKilled = 1;
                sys_kill_task(secondTaskId);
            }
        }

    }

    //se apreto ESC -> cerramos los programas y cerramos al runner
    sys_kill_task(firstTaskId);
    if (argc == 6)
    {
        sys_kill_task(secondTaskId);
        sys_clear_screen(BLACK);
    }
    sys_exit();
}


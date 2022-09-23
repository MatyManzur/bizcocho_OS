#include <scheduler.h>

typedef struct task_t //struct de info de cada task
{
    uint16_t taskId;        //identificador unico de cada task
    uint8_t screenId;        //identificador de la screen que está usando. Dos tasks pueden compartir la misma screen
    uint8_t active;        //si esta desactivado, se saca temporalmente de la cola de tasks a llamar (se pausa)
    uint8_t homeTask;        //si es una homeTask, cuando ya no queden mas tasks activas se reactiva automaticamente
    uint64_t stackPointer;        //valor del stackPointer que guarda para poder restablecer todos los registros al volver a esta task. Si es la primera vez que se llamó, stackPointer = 0
    const void (*initTask)(uint8_t argc, void **argv);    //puntero a funcion de la funcion que debe llamar la primera vez
    uint8_t argc;            //cantidad de argumentos que recibe la funcion a llamar
    void **argv;            //array de punteros a los argumentos que recibe la funcion
} task_t;

//Tenemos un array de tasks
static task_t taskArray[MAX_TASK_COUNT];
static uint8_t currentTaskCount = 0;

//taskIds a asignar. Arranca en 1 porque el 0 se usa para ver si el slot del array esta vacío
static uint16_t nextTaskId = 1;

//currentTaskIndex es el indice del array de la task actual. Arranca en -1 porque inicialmente no hay tasks
static int8_t currentTaskIndex = -1; //index es la posicion del array, id es el numero que se le da al usuario

//Funcion auxiliar que recibe el taskId y devuelve donde esta en el array de tasks, si no lo encuentra devuelve -1
static int8_t getTaskArrayIndex(uint16_t taskId)
{
    for (int i = 0; i < MAX_TASK_COUNT; i++)
    {
        if (taskArray[i].taskId == taskId)
            return i;
    }
    return -1;
}

//Funcion auxiliar que activa todas las tasks que son homeTasks
static void activateHomeTasks()
{
    for (int i = 0; i < MAX_TASK_COUNT; i++)
    {
        if (taskArray[i].homeTask)
        {
            taskArray[i].active = 1;
            currentTaskIndex = i;
        }
    }
}

//llamada por el timer_tick para que pase a la sgte task
void followingTask()
{
    if (currentTaskCount == 0)        //si no hay nada para correr, no hace nada
        return;
    uint8_t lastTaskIndex = currentTaskIndex; //se guarda en qué task estabamos anteriormente
    if (currentTaskIndex < 0)            //si nunca hubo una task devolvió -1, entonces que intente con la 0
        currentTaskIndex = 0;
    int i = 0;                //para ver si se dio una vuelta y no encontro nada activo
    //nos movemos hasta encontrar la proxima task que deberia correr
    do
    {
        i++;
        currentTaskIndex++;
        if (currentTaskIndex >=
            MAX_TASK_COUNT) //si nos pasamos de MAX_TASK_COUNT nos movemos al principio y seguimos incrementando
            currentTaskIndex = 0;
    } while (i <= MAX_TASK_COUNT &&
             (taskArray[currentTaskIndex].taskId == 0 || taskArray[currentTaskIndex].active == 0));
    if (i > MAX_TASK_COUNT) //si dio una vuelta y no había nada activo
    {
        activateHomeTasks(); //activa todos los hometask y mueve el currentTaskIndex a un homeTask
    }

    //Ahora taskArray[currentTaskIndex] es la proxima task a ejecutar

    if (lastTaskIndex >= 0) //si hubo una task previa
        saveStackPointer(&taskArray[lastTaskIndex].stackPointer); //deja en el puntero del argumento el rsp viejo

    if (taskArray[currentTaskIndex].stackPointer == 0) //si nunca se inicio esta task
    {
        initializeTask(taskArray[currentTaskIndex].argc, taskArray[currentTaskIndex].argv,
                       taskArray[currentTaskIndex].initTask,
                       TASKS_STACK_BASE - (currentTaskIndex + 1) * TASK_STACK_SIZE);
        //mueve el rsp a donde indica el 4to parametro, hace el EOI para el pic, y llama la funcion del 3er parametro con los primeros dos argumentos
        return;
    }
    swapTasks(taskArray[currentTaskIndex].stackPointer); //cambia el rsp al que le paso en el parametro
}

//Devuelve el screenId del task actual
int8_t getCurrentScreenId()
{
    if (currentTaskIndex < 0)
        return -1;
    return taskArray[currentTaskIndex].screenId;
}

//Devuelve el taskId del task actual
int8_t getCurrentTaskId()
{
    if (currentTaskIndex < 0)
        return -1;
    return taskArray[currentTaskIndex].taskId;
}

//Función auxiliar que agrega la task al array
static int16_t
addTaskToArray(const void (*initTask)(uint8_t argc, void **argv), uint8_t screenId, uint8_t homeTask, uint8_t argc,
               void **argv)
{
    if (currentTaskCount >= MAX_TASK_COUNT)
        return -1;
    int i = 0;
    currentTaskCount++;
    while (taskArray[i].taskId != 0) //nos movemos hasta encontrar un lugar vacio
        i++;
    taskArray[i] = (task_t) {.taskId = nextTaskId, .screenId = screenId, .active = 1, .homeTask = homeTask, .stackPointer = 0, .initTask = initTask, .argc = argc, .argv = argv};

    return nextTaskId++;
}

//Agrega una task al array con una nueva screen, devuelve -1 si no se pudo agregar
int16_t addTask(const void (*initTask)(uint8_t argc, void **argv), const struct point_t *topLeft,
                const struct point_t *bottomRight, uint8_t homeTask, uint8_t argc, void **argv)
{
    int8_t screenId = addScreenState(topLeft->row, topLeft->column, bottomRight->row, bottomRight->column);
    if (screenId < 0) //no hay mas screens disponibles
        return -1;
    return addTaskToArray(initTask, screenId, homeTask, argc, argv);
}

//Agrega una task al array con una screen compartida con otra task ya existente. Si no existe esa otra task, devuelve -1 y no la agrega al array
int16_t
addTaskWithSharedScreen(const void (*initTask)(uint8_t argc, void **argv), uint16_t otherTaskId, uint8_t homeTask,
                        uint8_t argc, void **argv)
{
    int8_t otherTaskIndex = getTaskArrayIndex(otherTaskId);
    if (otherTaskIndex < 0) //no existe esa other task
        return -1;
    return addTaskToArray(initTask, taskArray[otherTaskIndex].screenId, homeTask, argc, argv);
}

//activa la task con el taskId indicado, si no existia no hace nada
void activateTask(uint16_t taskId)
{
    int8_t taskIndex = getTaskArrayIndex(taskId);
    if (taskIndex < 0) //no existe esa task
        return;
    taskArray[taskIndex].active = 1;
}

//desactiva la task con el taskId indicado, si no existia no hace nada
void deactivateTask(uint16_t taskId)
{
    int8_t taskIndex = getTaskArrayIndex(taskId);
    if (taskIndex < 0) //no existe esa task
        return;
    taskArray[taskIndex].active = 0;
    followingTask();
}

//borra a la task indicada del taskArray
void killTask(uint16_t taskId)
{
    int8_t taskIndex = getTaskArrayIndex(taskId);
    if (taskIndex < 0) //no existe esa task
        return;
    currentTaskCount--;
    //buscamos si habían otras tasks con la misma screen
    int otherTasksWithSameScreen = 0;
    for (int i = 0; i < MAX_TASK_COUNT; i++)
    {
        if (i != taskIndex && taskArray[i].screenId == taskArray[taskIndex].screenId)
            otherTasksWithSameScreen++;
    }
    if (otherTasksWithSameScreen == 0)
    {
        deleteScreenState(taskArray[taskIndex].screenId);
    }
    taskArray[taskIndex] = (task_t) {0};
    followingTask();
}

//borra la task actual del taskArray
void exit()
{
    killTask(getCurrentTaskId());
}


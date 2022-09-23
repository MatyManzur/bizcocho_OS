#include <time.h>

static unsigned long ticks = 0; //cantidad total de ticks desde la primera interrupcion del timer tick

static struct timezone_t timezone = {0, 0}; //time zone actual con horas y minutos
static int monthdays[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
                               {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}}; //dias de cada mes en años bisiestos y no bisiestos

static int isLeapYear(int year);

//lo llama en la interrupcion del timer tick
void timer_handler()
{
    ticks++;
    followingTask();
}

//devuelve la cantidad total de ticks desde la primera interrupcion del timer tick
unsigned long ticks_elapsed()
{
    return ticks;
}

//devuelve la cantidad total de segundos desde la primera interrupcion del timer tick
unsigned long seconds_elapsed()
{
    return ticks / 18;
}

//devuelve por parametro la fecha y hora actual, y el timezone que se usó para devolverla
void getCurrentDateTime(struct datetime_t *datetime, struct timezone_t *tzone)
{
    tzone->hours = timezone.hours;
    tzone->minutes = timezone.minutes;
    int8_t h = ((int8_t) getHours()) + timezone.hours;
    int8_t m = ((int8_t) getMinutes()) + timezone.minutes;
    int8_t day = getDay();
    int8_t month = getMonth();
    uint16_t year = getYear() + getCentury() * 100;
    if (m < 0)
    {
        h--;
    }
    if (m >= 60)
    {
        h++;
    }
    if (h < 0)
    {
        if (day == 1)
        {
            month--;
            if (month == 0)
            {
                year--;
                month = 12;
            }
            day = monthdays[isLeapYear(year)][month - 1];
        } else
        {
            day--;
        }
    }
    if (h >= 24)
    {
        if (day == monthdays[isLeapYear(year)][month - 1])
        {
            month++;
            if (month > 12)
            {
                year++;
                month = 1;
            }
            day = 1;
        } else
        {
            day++;
        }
    }
    h += 24;
    h %= 24;
    m += 60;
    m %= 60;
    uint8_t s = getSeconds();
    datetime->hours = h;
    datetime->mins = m;
    datetime->segs = s;


    datetime->day = day;
    datetime->month = month;
    datetime->year = year;

}

//setea el timezone al indicado por parametro con horas y minutos
void setTimeZone(const struct timezone_t *tzone)
{
    timezone.hours = tzone->hours;
    timezone.minutes = tzone->minutes;
}

//Funcion auxiliar para ver si es un año bisiesto
static int isLeapYear(int year)
{
    if (year % 400 == 0)
    {
        return 1;
    } else if (year % 4 == 0 && year % 100 != 0)
    {
        return 1;
    }
    return 0;
}

//Espera a que hayan pasado la cantidad de ticks indicada
void sleep(uint64_t sleepTicks)
{
    uint64_t finish = ticks + sleepTicks;
    while (ticks < finish)
    {
        _hlt();    //espera a la proxima interrupcion. Tienen que llegar interrupciones del mismo timer tick para que incremente ticks y así salir de este while
    }
}

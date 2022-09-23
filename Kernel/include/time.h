#ifndef _TIME_H_
#define _TIME_H_

#include <lib.h>
#include <scheduler.h>

struct datetime_t
{
    uint8_t hours;
    uint8_t mins;
    uint8_t segs;

    uint8_t day;
    uint8_t month;
    uint16_t year;
};
struct timezone_t
{
    int8_t hours;
    int8_t minutes;
};

//rutina de atencion
void timer_handler();

//syscalls
unsigned long ticks_elapsed();

unsigned long seconds_elapsed();

void getCurrentDateTime(struct datetime_t *datetime, struct timezone_t *tzone);

void setTimeZone(const struct timezone_t *tzone);

void sleep(uint64_t sleepTicks);

//implementadas en time.asm
uint8_t getSeconds();

uint8_t getMinutes();

uint8_t getHours();

uint8_t getDay();

uint8_t getMonth();

uint8_t getYear();

uint8_t getCentury();


#endif

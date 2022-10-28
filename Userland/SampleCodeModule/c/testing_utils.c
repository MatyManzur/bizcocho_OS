
#include "testing_utils.h"

#define WAIT      10000000 // TODO: Change this value to make the wait long enough to see theese processes beeing run at least twice

static uint32_t m_z = 362436069;
static uint32_t m_w = 521288629;

uint32_t GetUint(){
    m_z = 36969 * (m_z & 65535) + (m_z >> 16);
    m_w = 18000 * (m_w & 65535) + (m_w >> 16);
    return (m_z << 16) + m_w;
}

uint32_t GetUniform(uint32_t max){
    uint32_t u = GetUint();
    return (u + 1.0) * 2.328306435454494e-10 * max;
}

void memseter(void * beginning, uint32_t value, uint32_t size)
{
    uint8_t *p = (uint8_t *) beginning;
    uint32_t i;

    for (i = 0; i < size; i++, p++)
        *p = value;
}

int memcheck(void * beginning, uint32_t value, uint32_t size)
{
    uint8_t *p = (uint8_t *) beginning;
    uint32_t i;

    for (i = 0; i < size; i++, p++)
    if (*p != value)
      return 0;

    return 1;
}

int64_t satoi(char* str){
  uint64_t i = 0;
  int64_t res = 0;
  int8_t sign = 1;

  if (!str) return 0;

  if (str[i] == '-'){
    i++;
    sign = -1;
  }

  for ( ; str[i] != '\0'; ++i){
    if(str[i] < '0' || str[i] > '9') return 0;
    res = res * 10 + str[i] - '0';
  }

  return res * sign;
}



void bussy_wait(uint64_t n){
  uint64_t i;
  for (i = 0; i < n; i++);
}

void endless_loop(){
  while(1);
}

void endless_loop_print()
{
  uint8_t pid = sys_get_pid();

  while(1){
    printf("%d\n", pid);
    bussy_wait(WAIT);
  }
}
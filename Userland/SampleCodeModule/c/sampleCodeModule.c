
#include <bizcocho.h>

int main()
{
    sys_start_parent_process("bizcocho", 0, NULL, bizcocho, 2);
    while(1);
    return 0;
}

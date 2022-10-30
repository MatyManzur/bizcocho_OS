
#include <bizcocho.h>

int main()
{
    sys_start_parent_process("bizcocho", 0, NULL, bizcocho, 0, 0);
    while(1);
    return 0;
}

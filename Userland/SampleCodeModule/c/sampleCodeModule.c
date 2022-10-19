
#include <bizcocho.h>

int main()
{
    // point_t topLeft = {0, 0};
    // point_t bottomRight = {24, 79};    // se le otorga toda la pantalla a bizcocho, nuestra shell
    // sys_add_task(bizcocho, &topLeft, &bottomRight, 1, 0, NULL);
    sys_start_parent_process("Bizcocho",0,NULL,bizcocho,2);
    return 0;
}

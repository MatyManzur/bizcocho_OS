#include <files.h>
//estas sí serían syscalls, no como las de printing.c

static void printToScreen(char* s,format_t* format){
    while(s!=NULL && *s != '\0'){
            s=print(s,format);
            if(s!=NULL){
                scrollUp(1);
            }
        }
}

int write(int fd,char* s)
{   
    format_t format;
    format.backgroundColor=DEFAULT;
    format.characterColor=DEFAULT;
    switch (fd)
    {
        case 0:
            //no se puede, el keyboard.c tiene el buffer
            return -1;
            break;

        case 1:
            printToScreen(s,&format);
            break;

        case 2:
            format.characterColor = WHITE;
            format.backgroundColor = RED;
            printToScreen(s,&format);
            break;
        default:
            break;
    }
    return 0;
}

uint8_t read(int fd, char* buf, uint8_t n)
{
    switch (fd)
    {
        case 0:
            int totalChars = 0;
            while(totalChars < n)
            {
                totalChars += readPrintables(buf, n - totalChars);
                if(totalChars < n)
                {
                    BlockedReason_t block;
                    block.id=0;
                    block.source=PIPE_READ;
                    blockProcessWithReason(getPid(),block);
                }
                
            }
            break;
        case 1:
        case 2:
            
            //No se puede
            return -1;
        break;
        
        default:
            break;
    }
}
/*
write(int fd, char* s)
- escribe hasta que encuentra un \0
- si el fd es 0 o 1 (stdin o stdout) llama a print de printing.c con DEFAULT, DEFAULT
- si es 2 (stderr) hace lo mismo pero con RED, WHITE
- dejamos que se pueda escribir en stdin para que lo pueda llamar el keyboard.c
- si es >2 despues vemos, por ahora q no haga nada
- al terminar de escribir sobre un fd, le tenemos que avisar al scheduler que desbloquee 
los procesos que estaban bloqueados por esta razon. 
- ver que onda el \n, lo manejamos acá, o lo manejamos en printing 
(creo que en printing seria mejor, en printChar)
- al llamar al print de printing.c, va a avisar cuando se termine la pantalla, en ese caso
llamar a scrollup()
*/

/*
- read(int fd, char* buf, int n)
- lee la cantidad n, o hasta que encuentre un \0 del fd indicado
- que no te deje leer de stdout o stderr
- hay q guardarnos un buffer de lo que está en stdin
- si todavia no leyo los n chars o no encontro un \0, le avisa al scheduler que bloquee el proceso 
con la reason de que está leyendo de ese fd
*/

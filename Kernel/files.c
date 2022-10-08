//estas sí serían syscalls, no como las de printing.c

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

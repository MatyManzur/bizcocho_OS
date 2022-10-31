#include <stringslib.h>

char *convert(unsigned int num, int base, unsigned int minDigitCount)
{
    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50];
    char *ptr;
    int digitCount = 0;

    ptr = &buffer[49];
    *ptr = '\0';

    do
    {
        *--ptr = Representation[num % base];
        num /= base;
        digitCount++;
    } while (num != 0);

    while (digitCount < minDigitCount) //agrega ceros adelante si faltan digits
    {
        *--ptr = Representation[0];
        digitCount++;
    }

    return (ptr);
}


//Extraídas de string.c
//Fuente: https://codebrowser.dev/linux/linux/lib/string.c.html
char *strncpy(char *dest, const char *src, size_t count)
{
	char *tmp = dest;
	while (count) {
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
}

int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;
	while (1) {
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

//variante de codigo sacado de:
//https://codebrowser.dev/linux/linux/lib/string.c.html
//Devuelve cuantos caracteres copió
size_t strcpy(char *dest, const char *src)
{
	size_t tmp = 0;
	while ((*dest++ = *src++) != '\0')
		tmp++;
	return tmp;
}


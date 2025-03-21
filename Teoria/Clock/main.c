#include <stdio.h>
#include <time.h>

void process()
{
    for(int i = 0; i < 1000000; i++)
    {
        // Do something
    }
}

int main()
{
    clock_t start, stop;
    start = clock();
    process();
    stop = clock();
    printf("%ld\n", stop - start);
    long tiempo = 1000000*(stop - start) / CLOCKS_PER_SEC; // Tiempo en microsegundos
    printf("%ld\n", tiempo);
    return 0;
}
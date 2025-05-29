#include <cstdio>
#include <windows.h>

int main()
{
    int Health = 100;
    printf("The player starts with 100 Health Points. Press H to do a 5 hit!\n");

    while (1)
    {
        if (GetAsyncKeyState('H') & 0x0001)
        {
            Health = Health - 5;
            printf("HP %d\n", Health);
        }

        if (Health <= 0)
        {
            printf("The Player died\n");
            return 0;
        }
    }
    return 0;
}

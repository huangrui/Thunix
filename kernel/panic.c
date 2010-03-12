#include <kernel.h>

void panic(const char * s)
{
        printk("Kernel panic: %s\n",s);
        for(;;)
                ;
}

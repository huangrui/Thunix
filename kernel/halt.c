/**
 *  thunix/kernel/halt.c
 *
 *  halt the computer
 *
 */

#include <asm/io.h>
#include <asm/system.h>
#include <thunix.h>


const char bochs_shutdown[] = "Shutdown";

void
halt (void)
{
        int i;

        cli();

        /* Bochs, QEMU, etc.  */
        for (i = 0; i < sizeof (bochs_shutdown) - 1; i++)
                outb (bochs_shutdown[i], 0x8900);
  
        panic ("Thunix doesn't know how to halt this machine yet!");
}

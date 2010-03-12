/**
 *  thunix/kernel/reboot.c
 *
 *  reboot the system
 *
 */

#include <thunix.h>
#include <asm/io.h>


#define KEYBOARD_REG_STATUS	0x64
#define KEYBOARD_COMMAND_REBOOT	0xfe


void reboot()
{
        outb_p(KEYBOARD_COMMAND_REBOOT, KEYBOARD_REG_STATUS);

        /* if fails, then print error message */
        /* hope it never happens */
        panic("Sorry, reboot doesn't supported...");
}

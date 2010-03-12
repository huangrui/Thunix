/**
 *  thunix/kernel/shell.c
 *
 *  The shell part, it just supported a limit number of commands now.
 *   
 *  Copyright (C) Aleaxander 2008-2009
 *                
 *                Aleaxander@gmail.com
 */

#include <thunix.h>
#include <time.h>
#include <console.h>

#define BUFFER_SIZE 512

char buf[BUFFER_SIZE];
char *version_string = "v0.2";


int is_command(char *com_buf, char *command)
{
        if ( strcmp(com_buf, command) == 0)
                return 1;
        else
                return 0;
}


void about()
{
        puts("\n");
        printk("\tthunix, a 32-bit operating system, made by Alexander.\n");
        printk("\tmostly written by c and little asm language.\n");
        printk("\tthe lastest version is thunix-0.2\n");
        printk("\tthe website is http://www.osdever.cn\n");
        puts("\n");
        printk("\tas the ram_fs we used, that's to say our all data crated.\n");
        printk("\twill cleard after the shutdown\n");
        printk("\tso, it's just a test system, Yeah, that's it!\n");
        printk("\n\tThat's it ...\n");
        puts("\n");
}

void help()
{
        puts("\n");
        printk("%s%s%s%s%s%s%s%s%s%s%s%s","thunix, version 0.2\n",      \
               "\n",                                                    \
               "about		something about thunix\n",              \
               "clear           clear the screen\n",                    \
               "date            display the current date\n",            \
               "help		will print this page\n",                \
               "mkdir           will make a new directory\n",           \
               "ls 		will list the '/' root driectory\n",    \
               "halt            shut down the computer\n",              \
               "hello		a test command, just print 'hello thunix'\n", \
               "reboot          reboot the computer\n",                 \
               "version         print the current version\n"            \
               );
        puts("\n");
}

void version()
{
        printk("%s\n",version_string);
}

void date()
{
        struct tm time;
        get_current_time(&time);
        
        printk("%d/%02d/%02d %d:%d:%d\n",                               \
               time.tm_year+2000, time.tm_mon,                          \
               time.tm_mday,time.tm_hour,                               \
               time.tm_min, time.tm_sec                                 \
               );
}


extern void cls(void);
extern void ls(char *, char *);
extern void reboot();
extern void mkdir(char *);
extern void halt(void);
void run_command(char *command, int argc, char **argv)
{
        
        
        if ( is_command(command, "about")  )
                about();

        else if (is_command(command, "clear") )
                cls();

        else if (is_command(command, "date") )
                date();

        else if (is_command(command, "halt") )
                halt();

        else if ( is_command(command, "hello") )
                printk("hello thunix\n");

        else if ( is_command(command, "help") )
                help();

        else if ( is_command(command, "mkdir") ) {
                if (argc < 1) {
                        printk("mkdir usage: please input at least one dir name\n");
                } else {
                        while ( argc ) {
                                argc --;
                                mkdir(argv[argc]);
                        }
                }
        }
        
        else if ( is_command(command, "ls") ) {
                if ( argc == 0) {
                        ls ("/", "-l");
                } else if (argc == 1) {
                        ls(argv[0], 0);
                } else {
                        int i = 0;
                        while ( i < argc) {
                                printk("%s:\n", argv[i]);
                                ls (argv[i], "-l");
                                i ++;
                        }
                }
        }

        else if ( is_command(command, "version") )
                version ();

        else if ( is_command(command, "reboot") )
                reboot ();
        else
                printk("unknown command, please type 'help' for more information\n");
}


void debug_command(char *command, int argc, char **argv)
{
        int i = 0;
        printk("command = %s\n", command);

        while ( i < argc) {
                printk("argv[%d] = %s\n", i, argv[i]);
                i ++;
        }
}

        


/* For now it's enough */
#define MAX_OPTION 4
void parse_command(char * command_buffer)
{
        char *command;
        char *argv[MAX_OPTION];
        char *p;
        int argc = 0; 

        command = command_buffer;

        p = command;
        
 again:    
        while ( (*p != ' ') && (*p) )
                p ++;
        if ( *p == '\0' )
                goto out;

        *p = '\0';
        p ++;
        while (*p == ' ')
                p ++;
        
        command_buffer = p;
        argv[argc] = command_buffer;
        argc ++;
        goto again;
 out:
        //debug_command(command, argc, argv);

        run_command(command, argc, argv);
}








void shell_init()
{
        puts("thunix $ ");

}

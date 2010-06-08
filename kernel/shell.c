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
#include <stdio.h>
#include <string.h>

#include <tfs.h>
#include <hexdump.h>

#define BUFFER_SIZE 512


char buf[BUFFER_SIZE];
static char *version_string = "v0.4";


static int atoi_hex(char *str)
{
	int sum = 0;

	while (*str) {
		int rest;

		rest = *str < '9' ? *str - '0' : (*str | 0x20) - 'a';
		sum = sum * 16 + rest;
		str++;
	}

	return sum;
}

static int atoi(char *str)
{
	int sum = 0;

	if (str[0] == '0' && (str[1] | 0x20) == 'x')
		return atoi_hex(str + 2);

	while (*str) {
		sum = sum * 10 + *str - '0';
		str++;
	}
	return sum;
}
	
static int try_hex(char *str)
{
	while (*str) {
		if ((*str >= '0' && *str <= '9') || 
		    ((*str | 0x20) >= 'a' && (*str | 0x20) <= 'f')) {
			str++;
			continue;
		}

		return 0;
	}

	return 1;
}	

/* 
 * Test if the string is number or not, return 1 if true or 0
 * 
 * It just recongnize hexdump and normal digits now
 */
static int is_digits(char *str)
{

	if (str[0] == '0' && (str[1] | 0x20) == 'x')
		if (try_hex(str + 2))
			return 1;

	while (*str) {
		if (*str > '9' || *str < '0')
			return 0;
		str++;
	}

	return 1;
}

static int is_command(char *com_buf, char *command)
{
        if ( strcmp(com_buf, command) == 0)
                return 1;
        else
                return 0;
}


static void about()
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

static void help()
{
        puts("\n");
        printk("%s%s%s%s%s%s%s%s%s%s%s%s%s","thunix, version 0.2\n",      \
               "\n",                                                    \
               "about		something about thunix\n",              \
               "clear           clear the screen\n",                    \
               "date            display the current date\n",            \
               "help		will print this page\n",                \
	       "hexdump		dump the data or file as hex number\n", \
               "mkdir           will make a new directory\n",           \
               "ls 		will list the '/' root driectory\n",    \
               "halt            shut down the computer\n",              \
               "hello		a test command, just print 'hello thunix'\n", \
               "reboot          reboot the computer\n",                 \
               "version         print the current version\n"            \
               );
        puts("\n");
}

static void version()
{
        printk("%s\n",version_string);
}

static void date()
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
extern void reboot();
extern void ls(struct tfs_sb_info *sbi, char *);
extern void cd(struct tfs_sb_info *sbi, char *);
extern void cp(struct tfs_sb_info *sbi, char *, char *);
extern void cat(struct tfs_sb_info *sbi, char *);
extern void mkdir(struct tfs_sb_info *sbi, char *);
extern void rmdir(struct tfs_sb_info *sbi, char *);
extern void rm(struct tfs_sb_info *sbi, char *);
extern void touch(struct tfs_sb_info *sbi, char *);
extern void halt(void);
void run_command(char *command, int argc, char **argv)
{
        
        
        if ( is_command(command, "about")  )
                about();
		
	else if ( is_command(command, "cat") )
		cat(tfs_sbi, argv[1]);
	else if ( is_command(command, "cd") )
		cd(tfs_sbi, argv[1]);

        else if (is_command(command, "clear") )
                cls();
	else if (is_command(command, "cp") )
		cp(tfs_sbi, argv[1], argv[2]);

        else if (is_command(command, "date") )
                date();

        else if (is_command(command, "halt") )
                halt();

        else if ( is_command(command, "hello") )
                printk("hello thunix\n");

        else if ( is_command(command, "help") )
                help();

	else if ( is_command(command, "hexdump") ) {
		if (argc != 3) {
			printk("hexdump usage: hexdump 'mem_addr or string' length\n");
		} else {
			if(is_digits(argv[1]))
				hexdump((char *)atoi(argv[1]), atoi(argv[2]));
			else {
				printk("Not a valid memory addrss, try hexdump it as string\n");
				hexdump(argv[1], atoi(argv[2]));
			}
		}
	}

        else if ( is_command(command, "mkdir") ) {
                if (argc != 2) {
                        printk("mkdir usage: mkdir dir\n");
                } else {
			mkdir(tfs_sbi, argv[1]);	
                }
        }
	else if ( is_command(command, "rmdir") ) {
		rmdir(tfs_sbi, argv[1]);
	} 

	else if ( is_command(command, "rm") ) {
		rm(tfs_sbi, argv[1]);

	}
	else if ( is_command(command, "touch") ) {
		touch(tfs_sbi, argv[1]);
	}
        
        else if ( is_command(command, "ls") ) {
		if (argc == 1)
			argv[1] = ".";
		ls(tfs_sbi, argv[1]);
        }

        else if ( is_command(command, "version") )
                version ();

        else if ( is_command(command, "reboot") )
                reboot ();
#if 1
	else if ( is_command(command, "debug") )
		Debug();
#endif
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
        char *argv[MAX_OPTION] = {};
        char *p;
        int argc = 0; 

        command = command_buffer;

#if 0
	printk("cmdline: %s\n", command);
	hexdump(command, strlen(command));
#endif

        p = command;
        
        while ( (*p != ' ') && (*p) )
                p ++;
	argv[argc++] = command;

	while (*p && argc < MAX_OPTION) {
                *p = '\0';
		p++;
                argv[argc++] = p;
		while (*p && *p != ' ')
			p++;
                while (*p && *p == ' ')
                        p++;
	        if ( *p == '\0' )
       			break;
		p--;
                
	}

        run_command(command, argc, argv);
}








void shell_init()
{
        puts("thunix $ ");

}

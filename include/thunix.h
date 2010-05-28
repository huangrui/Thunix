#ifndef _THUNIX_H
#define _THUNIX_H

#include <time.h>

#define pause() ({                              \
                        while (1)               \
                                ;               \
                })

#define DEBUG_ON 1
#define EXT2_DEBUG_ON 0

#define DEBUG(x) ( {							\
                        if (DEBUG_ON) {                                 \
                                printk("DEBUG (%s, %d): %s:",__FILE__,__LINE__,__FUNCTION__); \
                                x;                                      \
                        }                                               \
                })

#define EXT2_DEBUG(x) ( {                                               \
                        if (EXT2_DEBUG_ON) {                            \
                                printk("DBG (%s, %d): %s: ",            \
                                       __FILE__, __LINE__, __FUNCTION__); \
                                x;                                      \
                        }                                               \
                })

#define CURRENT_TIME    0  /* Not yet */

/* for fs part */
#define RAM_EXT2_FS 1



extern void panic(char *);

extern unsigned long get_current_time(struct tm*);

#define LOG_PRINT printk

/* General debug function */
extern void Debug(void);

#endif /* thunix.h */

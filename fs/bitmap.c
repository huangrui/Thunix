/*
 * thunix/fs/bitmap.c
 *
 * something about bit set, clear or test
 */


void ext2_set_bit(void *addr, unsigned int i)
{
        unsigned char *v = addr;
        v += i>>3;
        *v |= 1<<(7-(i%8));
}

void ext2_clear_bit(void *addr, unsigned int i)
{
        unsigned char *v = addr;
        v += i>>3;
        *v &= ~(1<<(7-(i%8)));
}

int ext2_test_bit(void *addr, unsigned int i)
{
        unsigned char *v = addr;
        v += i>>3;
        return (*v & (1<<(7-(i%8)))) != 0;
}

int find_first_zero(void *addr, unsigned int offset, unsigned int max)
{
        unsigned char *v = addr;
        int i = offset;
        while(i < max - 1) {
                if (!ext2_test_bit(v, i))
                        return i;
                i++;
        }
        
        return 0;  
}

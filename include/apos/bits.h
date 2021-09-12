#ifndef APOS_BITS_H
#define APOS_BITS_H

#define __is_set(x, y) ((x) & (y))
#define __set_bit(x, y) ((x) |= (y))
#define __clear_bit(x, y) ((x) &= ~(y))

#define __is_nset(x, y) (__is_set((x), 1 << (y)))
#define __set_nbit(x, y) (__set_bit((x), 1 << (y)))
#define __clear_nbit(x, y) (__clear_bit((x), 1 << (y)))

#endif /* APOS_BITS_H */

#ifndef APOS_BITS_H
#define APOS_BITS_H

#define __is_set(x, y) ((x) & (y))
#define __set_bit(x, y) ((x) |= (y))
#define __clear_bit(x, y) ((x) &= ~(y))

#endif /* APOS_BITS_H */

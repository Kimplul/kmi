#ifndef KMI_BKL_H
#define KMI_BKL_H

#include <kmi/lock.h>

extern spinlock_t bkl;

static inline void bkl_lock()
{
	spin_lock(&bkl);
}

static inline void bkl_unlock()
{
	spin_unlock(&bkl);
}

#endif /* KMI_BKL_H */

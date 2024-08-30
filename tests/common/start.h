#ifndef KMI_START_H
#define KMI_START_H

#define UNUSED(x) (void)x
#define START(pid, tid, d0, d1, d2, d3)\
void _start(sys_arg_t pid, sys_arg_t tid,\
		sys_arg_t d0, sys_arg_t d1, sys_arg_t d2, sys_arg_t d3)

#endif /* KMI_START_H */

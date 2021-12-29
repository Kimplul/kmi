#include <apos/tcb.h>
#include <apos/cpu.h>
#include <apos/utils.h>
#include <apos/sp_tree.h>

static struct sp_root t_root = (struct sp_root){0};
static struct tcb *__tcb_cache[MAX_CPUS] = {0};

#define tcb_container(x) \
	container_of(x, struct tcb, sp_n)

void threads_insert(struct tcb *t)
{
	if(!sp_root(t_root)){
		sp_root(t_root) = &t->sp_n;
		return;
	}

	struct sp_node *n = sp_root(t_root), *p = NULL;
	enum sp_dir d = LEFT;

	while(n){
		struct tcb *tc = tcb_container(n);
		p = n;

		if(t->tid < tc->tid){
			n = sp_left(n);
			d = LEFT;
		}

		else {
			n = sp_right(n);
			d = RIGHT;
		}
	}

	sp_insert(&sp_root(t_root), p, &t->sp_n, d);
}

struct tcb *threads_find(id_t tid)
{
	struct sp_node *n = sp_root(t_root);

	while(n){
		struct tcb *t = tcb_container(n);

		if(t->tid == tid)
			return t;

		if(t->tid < tid)
			n = sp_left(n);
		else
			n = sp_right(n);
	}

	return 0;
}

struct tcb *get_tcb(id_t pid)
{
	/* TODO: figure out how exactly thread IDs are related to process IDs,
	 * and figure out mapping between them. */
	return 0;
}

struct tcb *cur_tcb()
{
	return __tcb_cache[cpu_id()];
}

void set_tcb(struct tcb *t)
{
	__tcb_cache[cpu_id()] = t;
}

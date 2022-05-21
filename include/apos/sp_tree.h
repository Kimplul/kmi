#ifndef SP_TREE_H
#define SP_TREE_H

#include <apos/types.h>

#define sp_root(r)       ((r)->sp_r)
#define sp_left(n)       ((n)->left)
#define sp_right(n)      ((n)->right)
#define sp_rparen(n)     (sp_right(n)->parent)
#define sp_lparen(n)     (sp_left(n)->parent)
#define sp_paren(n)      ((n)->parent)
#define sp_gparen(n)     ((n)->parent->parent)
#define sp_has_gparen(n) (sp_paren(n) && sp_gparen(n))

struct sp_node {
	int_fast16_t hint;
	struct sp_node *left;
	struct sp_node *right;
	struct sp_node *parent;
};

struct sp_root {
	struct sp_node *sp_r;
};

enum sp_dir { LEFT, RIGHT };

struct sp_node *sp_first(struct sp_node *n);
struct sp_node *sp_last(struct sp_node *n);

void sp_insert(struct sp_node **root, struct sp_node *p, struct sp_node *n,
               enum sp_dir d);

void sp_remove(struct sp_node **root, struct sp_node *n);

#endif /* SP_TREE_H */

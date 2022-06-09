#ifndef SP_TREE_H
#define SP_TREE_H

/**
 * @file sp_tree.h
 * sp_trees, a type of binary search trees.
 */

#include <apos/types.h>

/**
 * Get root of tree from \ref sp_root.
 *
 * @param r Instance of \ref sp_root.
 * @return Actual root of tree.
 */
#define sp_root(r) ((r)->sp_r)

/**
 * Get left node.
 *
 * @param n Node to read.
 * @return Left node of read node.
 * \see sp_right().
 */
#define sp_left(n) ((n)->left)

/**
 * Get right node.
 *
 * @param n Node to read.
 * @return Right node of read node.
 * \see sp_left().
 */
#define sp_right(n) ((n)->right)

/**
 * Get parent of right node.
 * In most situations should point back towards the node it started from, but
 * when in the middle of updating the tree it might temporarily point somewhere else.
 *
 * @param n Node to read.
 * @return Parent of right node of read node.
 * \see sp_lparen().
 */
#define sp_rparen(n) (sp_right(n)->parent)

/**
 * Get parent of left node.
 *
 * @param n Node to read.
 * @return Parent of left node of read node.
 * \see sp_rparen().
 */
#define sp_lparen(n) (sp_left(n)->parent)

/**
 * Get parent of node.
 *
 * @param n Node to read.
 * @return Parent of read node.
 */
#define sp_paren(n) ((n)->parent)

/**
 * Get grandparent (parent of parent) of node.
 *
 * @param n Node to read.
 * @return Grandparent of read node.
 * \see sp_has_gparen().
 */
#define sp_gparen(n) ((n)->parent->parent)

/** Check if node has grandparent.
 *
 * @param n Node to read.
 * @return Non-zero if node has grandparent, \c 0 otherwise.
 * \see sp_gparen().
 */
#define sp_has_gparen(n) (sp_paren(n) && sp_gparen(n))

/** Tree node.
 * Embed this structure in structures you want to build a tree of.
 * \see common/mem_region.c, for example.
 */
struct sp_node {
	/** Hint. Approximate maximum tree height up to the current node. */
	int_fast16_t hint;

	/** Lefthand node. */
	struct sp_node *left;

	/** Righthand node. */
	struct sp_node *right;

	/** Parent node. Technically speaking not necessary, but in this case I
	 * went with time over space. */
	struct sp_node *parent;
};

/** Convenience structure for trees. */
struct sp_root {
	/** Pointer to actual root of tree. */
	struct sp_node *sp_r;
};

/** Which side of the parent node a new node should be inserted to. */
enum sp_dir {
	/** Left side. */
	LEFT,

	/** Right side. */
	RIGHT
};

/**
 * Get first, leftmost node under specified node.
 *
 * @param n Node to start with.
 * @return Leftmost node under \c n, or \c n if there are none.
 */
struct sp_node *sp_first(struct sp_node *n);

/**
 * Get last, rightmost node under specified node.
 *
 * @param n Node to start with.
 * @return Rightmost node under \c n, or \c n if there are none.
 */
struct sp_node *sp_last(struct sp_node *n);

/**
 * Insert new node into tree.
 * Does not allocate any memory.
 *
 * @param root Root of tree.
 * @param p Parent of new node.
 * @param n New node.
 * @param d Which side of the parent node the new node should be on.
 */
void sp_insert(struct sp_node **root, struct sp_node *p, struct sp_node *n,
               enum sp_dir d);

/**
 * Remove node from tree.
 * Does not free any memory.
 *
 * @param root Root of tree.
 * @param n Node to remove.
 */
void sp_remove(struct sp_node **root, struct sp_node *n);

#endif /* SP_TREE_H */

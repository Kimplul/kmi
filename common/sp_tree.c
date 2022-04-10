#include <apos/sp_tree.h>

static void __sp_turn_left(struct sp_node *n)
{
	struct sp_node *l = sp_left(n);
	struct sp_node *p = sp_paren(n);

	sp_paren(l) = sp_paren(n);
	sp_left(n) = sp_right(l);
	sp_paren(n) = l;
	sp_right(l) = n;

	if (p && sp_left(p) == n)
		sp_left(p) = l;
	else if (p)
		sp_right(p) = l;

	if (sp_left(n))
		sp_lparen(n) = n;
}

static void __sp_turn_right(struct sp_node *n)
{
	struct sp_node *r = sp_right(n);
	struct sp_node *p = sp_paren(n);

	sp_paren(r) = sp_paren(n);
	sp_right(n) = sp_left(r);
	sp_paren(n) = r;
	sp_left(r) = n;

	if (p && sp_left(p) == n)
		sp_left(p) = r;
	else if (p)
		sp_right(p) = r;

	if (sp_right(n))
		sp_rparen(n) = n;
}

static int __sp_balance(struct sp_node *n)
{
	int l = 0;
	int r = 0;

	if (sp_left(n))
		l = sp_left(n)->hint + 1;

	if (sp_right(n))
		r = sp_right(n)->hint + 1;

	return l - r;
}

static int __sp_max_hint(struct sp_node *n)
{
	int l = 0;
	int r = 0;

	if (sp_left(n))
		l = sp_left(n)->hint + 1;

	if (sp_right(n))
		r = sp_right(n)->hint + 1;

	if (l > r)
		return l;
	else
		return r;
}

static void __sp_update(struct sp_node **root, struct sp_node *n)
{
	while (n) {
		int b = __sp_balance(n);
		int prev_hint = n->hint;
		struct sp_node *p = sp_paren(n);

		if (b < -1) {
			/* leaning to the right */
			if (n == *root)
				*root = sp_right(n);

			__sp_turn_right(n);
		}

		else if (b > 1) {
			/* leaning to the left */
			if (n == *root)
				*root = sp_left(n);

			__sp_turn_left(n);
		}

		n->hint = __sp_max_hint(n);
		if (n->hint == 0 || n->hint != prev_hint)
			n = p;
		else
			return;
	}
}

void sp_insert(struct sp_node **root, struct sp_node *p, struct sp_node *n,
               enum sp_dir d)
{
	if (!*root) {
		*root = n;
		return;
	}

	if (d == LEFT)
		sp_left(p) = n;
	else
		sp_right(p) = n;

	sp_paren(n) = p;
	__sp_update(root, n);
}

static void __sp_replace_right(struct sp_node *n, struct sp_node *r)
{
	struct sp_node *p = sp_paren(n);
	struct sp_node *rp = sp_paren(r);

	if (sp_left(rp) == r) {
		sp_left(rp) = sp_right(r);
		if (sp_right(r))
			sp_rparen(r) = rp;
	}

	if (sp_paren(rp) == n)
		sp_paren(rp) = r;

	sp_paren(r) = p;
	sp_left(r) = sp_left(n);

	if (sp_right(n) != r) {
		sp_right(r) = sp_right(n);
		sp_rparen(n) = r;
	}

	if (p && sp_left(p) == n)
		sp_left(p) = r;
	else if (p)
		sp_right(p) = r;

	if (sp_left(n))
		sp_lparen(n) = r;
}

static void __sp_replace_left(struct sp_node *n, struct sp_node *l)
{
	struct sp_node *p = sp_paren(n);
	struct sp_node *lp = sp_paren(l);

	if (sp_right(lp) == l) {
		sp_right(lp) = sp_left(l);
		if (sp_left(l))
			sp_lparen(l) = lp;
	}

	if (sp_paren(lp) == n)
		sp_paren(lp) = l;

	sp_paren(l) = p;
	sp_right(l) = sp_right(n);

	if (sp_left(n) != l) {
		sp_left(l) = sp_left(n);
		sp_lparen(n) = l;
	}

	if (p && sp_left(p) == n)
		sp_left(p) = l;
	else if (p)
		sp_right(p) = l;

	if (sp_right(n))
		sp_rparen(n) = l;
}

/* TODO: handle root better */
void sp_remove(struct sp_node **root, struct sp_node *del)
{
	if (sp_right(del)) {
		struct sp_node *least = sp_first(sp_right(del));

		if (del == *root)
			*root = least;

		__sp_replace_right(del, least);
		__sp_update(root, sp_right(least));
		return;
	}

	if (sp_left(del)) {
		struct sp_node *most = sp_last(sp_left(del));

		if (del == *root)
			*root = most;

		__sp_replace_left(del, most);
		__sp_update(root, sp_left(most));
		return;
	}

	if (del == *root) {
		*root = 0;
		return;
	}

	/* empty node */
	struct sp_node *paren = sp_paren(del);

	if (sp_left(paren) == del)
		sp_left(paren) = 0;
	else
		sp_right(paren) = 0;

	__sp_update(root, paren);
}

struct sp_node *sp_first(struct sp_node *n)
{
	if (!sp_left(n))
		return n;

	return sp_first(sp_left(n));
}

struct sp_node *sp_last(struct sp_node *n)
{
	if (!sp_right(n))
		return n;

	return sp_last(sp_right(n));
}

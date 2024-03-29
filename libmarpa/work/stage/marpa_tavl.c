/* Produced by texiweb from libavl.w. */

/* libavl - library for manipulation of binary trees.
   Copyright (C) 1998, 1999, 2000, 2001, 2002, 2004 Free Software
   Foundation, Inc.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301 USA.
*/


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* I changed the memory allocator from a parameter to a global.  Not
 * thread-safe, but it is only used in the test, which is not threaded.
 * In production Marpa's * |my_malloc| is hardwired in, by the following defines,
 * and that is thread-safe.
 */

#ifndef TESTING_TAVL
#define TESTING_TAVL 0
#endif /* TESTING_TAVL */

#if TESTING_TAVL

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

/* First parameter is really not needed */
#define MY_AVL_MALLOC(size) \
  (marpa__tavl_allocator_default->libavl_malloc(marpa__tavl_allocator_default, (size)))
#define MY_AVL_FREE(p) \
  (marpa__tavl_allocator_default->libavl_free(marpa__tavl_allocator_default, (p)))

#else /* not TESTING_TAVL */

#include "config.h"

#include <assert.h>
#include <stdio.h>

#include "marpa.h"
#include "marpa_ami.h"

#define MY_AVL_MALLOC(size) (my_malloc(size))
#define MY_AVL_FREE(p) (my_free(p))

#endif /* TESTING AVL */

#include "marpa_tavl.h"

/* Creates and returns a new table
   with comparison function |compare| using parameter |param|
   and memory allocator |allocator|.
   Returns |NULL| if memory allocation failed. */
MARPA_TAVL_LINKAGE struct tavl_table *
marpa__tavl_create (tavl_comparison_func *compare, void *param)
{
  struct tavl_table *tree;

  assert (compare != NULL);

  tree = MY_AVL_MALLOC( sizeof *tree);

  tree->tavl_root = NULL;
  tree->tavl_compare = compare;
  tree->tavl_param = param;
  tree->tavl_count = 0;

  return tree;
}

/* Search |tree| for an item matching |item|, and return it if found.
   Otherwise return |NULL|. */
MARPA_TAVL_LINKAGE void *
marpa__tavl_find (const struct tavl_table *tree, const void *item)
{
  const struct tavl_node *p;

  assert (tree != NULL && item != NULL);

  p = tree->tavl_root;
  if (p == NULL)
    return NULL;

  for (;;)
    {
      int cmp, dir;

      cmp = tree->tavl_compare (item, p->tavl_data, tree->tavl_param);
      if (cmp == 0)
        return p->tavl_data;

      dir = cmp > 0;
      if (p->tavl_tag[dir] == TAVL_CHILD)
        p = p->tavl_link[dir];
      else
        return NULL;
    }
}

/* Inserts |item| into |tree| and returns a pointer to |item|'s address.
   If a duplicate item is found in the tree,
   returns a pointer to the duplicate without inserting |item|.
   Returns |NULL| in case of memory allocation failure. */
MARPA_TAVL_LINKAGE void **
marpa__tavl_probe (struct tavl_table *tree, void *item)
{
  struct tavl_node *y, *z; /* Top node to update balance factor, and parent. */
  struct tavl_node *p, *q; /* Iterator, and parent. */
  struct tavl_node *n;     /* Newly inserted node. */
  struct tavl_node *w;     /* New root of rebalanced subtree. */
  int dir;                /* Direction to descend. */

  unsigned char da[TAVL_MAX_HEIGHT]; /* Cached comparison results. */
  int k = 0;              /* Number of cached results. */

  assert (tree != NULL && item != NULL);
  tree->tavl_duplicate_found = 0; /* Default to no duplicate found */

  z = (struct tavl_node *) &tree->tavl_root;
  y = tree->tavl_root;
  if (y != NULL)
    {
      for (q = z, p = y; ; q = p, p = p->tavl_link[dir])
        {
          int cmp = tree->tavl_compare (item, p->tavl_data, tree->tavl_param);
          if (cmp == 0) {
	    tree->tavl_duplicate_found = 1;
            return &p->tavl_data;
	  }

          if (p->tavl_balance != 0)
            z = q, y = p, k = 0;
          dir = cmp > 0;
          da[k++] = (unsigned char)dir;

          if (p->tavl_tag[dir] == TAVL_THREAD)
            break;
        }
    }
  else
    {
      p = z;
      dir = 0;
    }

  n = MY_AVL_MALLOC( sizeof *n);

  tree->tavl_count++;
  n->tavl_data = item;
  n->tavl_tag[0] = n->tavl_tag[1] = TAVL_THREAD;
  n->tavl_link[dir] = p->tavl_link[dir];
  if (tree->tavl_root != NULL)
    {
      p->tavl_tag[dir] = TAVL_CHILD;
      n->tavl_link[!dir] = p;
    }
  else
    n->tavl_link[1] = NULL;
  p->tavl_link[dir] = n;
  n->tavl_balance = 0;
  if (tree->tavl_root == n)
    return &n->tavl_data;

  for (p = y, k = 0; p != n; p = p->tavl_link[da[k]], k++)
    if (da[k] == 0)
      p->tavl_balance--;
    else
      p->tavl_balance++;

  if (y->tavl_balance == -2)
    {
      struct tavl_node *x = y->tavl_link[0];
      if (x->tavl_balance == -1)
        {
          w = x;
          if (x->tavl_tag[1] == TAVL_THREAD)
            {
              x->tavl_tag[1] = TAVL_CHILD;
              y->tavl_tag[0] = TAVL_THREAD;
              y->tavl_link[0] = x;
            }
          else
            y->tavl_link[0] = x->tavl_link[1];
          x->tavl_link[1] = y;
          x->tavl_balance = y->tavl_balance = 0;
        }
      else
        {
          assert (x->tavl_balance == +1);
          w = x->tavl_link[1];
          x->tavl_link[1] = w->tavl_link[0];
          w->tavl_link[0] = x;
          y->tavl_link[0] = w->tavl_link[1];
          w->tavl_link[1] = y;
          if (w->tavl_balance == -1)
            x->tavl_balance = 0, y->tavl_balance = +1;
          else if (w->tavl_balance == 0)
            x->tavl_balance = y->tavl_balance = 0;
          else /* |w->tavl_balance == +1| */
            x->tavl_balance = -1, y->tavl_balance = 0;
          w->tavl_balance = 0;
          if (w->tavl_tag[0] == TAVL_THREAD)
            {
              x->tavl_tag[1] = TAVL_THREAD;
              x->tavl_link[1] = w;
              w->tavl_tag[0] = TAVL_CHILD;
            }
          if (w->tavl_tag[1] == TAVL_THREAD)
            {
              y->tavl_tag[0] = TAVL_THREAD;
              y->tavl_link[0] = w;
              w->tavl_tag[1] = TAVL_CHILD;
            }
        }
    }
  else if (y->tavl_balance == +2)
    {
      struct tavl_node *x = y->tavl_link[1];
      if (x->tavl_balance == +1)
        {
          w = x;
          if (x->tavl_tag[0] == TAVL_THREAD)
            {
              x->tavl_tag[0] = TAVL_CHILD;
              y->tavl_tag[1] = TAVL_THREAD;
              y->tavl_link[1] = x;
            }
          else
            y->tavl_link[1] = x->tavl_link[0];
          x->tavl_link[0] = y;
          x->tavl_balance = y->tavl_balance = 0;
        }
      else
        {
          assert (x->tavl_balance == -1);
          w = x->tavl_link[0];
          x->tavl_link[0] = w->tavl_link[1];
          w->tavl_link[1] = x;
          y->tavl_link[1] = w->tavl_link[0];
          w->tavl_link[0] = y;
          if (w->tavl_balance == +1)
            x->tavl_balance = 0, y->tavl_balance = -1;
          else if (w->tavl_balance == 0)
            x->tavl_balance = y->tavl_balance = 0;
          else /* |w->tavl_balance == -1| */
            x->tavl_balance = +1, y->tavl_balance = 0;
          w->tavl_balance = 0;
          if (w->tavl_tag[0] == TAVL_THREAD)
            {
              y->tavl_tag[1] = TAVL_THREAD;
              y->tavl_link[1] = w;
              w->tavl_tag[0] = TAVL_CHILD;
            }
          if (w->tavl_tag[1] == TAVL_THREAD)
            {
              x->tavl_tag[0] = TAVL_THREAD;
              x->tavl_link[0] = w;
              w->tavl_tag[1] = TAVL_CHILD;
            }
        }
    }
  else
    return &n->tavl_data;
  z->tavl_link[y != z->tavl_link[0]] = w;

  return &n->tavl_data;
}

/* Returns the parent of |node| within |tree|,
   or a pointer to |tavl_root| if |s| is the root of the tree. */
static struct tavl_node *
find_parent (struct tavl_table *tree, struct tavl_node *node)
{
  if (node != tree->tavl_root)
    {
      struct tavl_node *x, *y;

      for (x = y = node; ; x = x->tavl_link[0], y = y->tavl_link[1])
        if (y->tavl_tag[1] == TAVL_THREAD)
          {
            struct tavl_node *p = y->tavl_link[1];
            if (p == NULL || p->tavl_link[0] != node)
              {
                while (x->tavl_tag[0] == TAVL_CHILD)
                  x = x->tavl_link[0];
                p = x->tavl_link[0];
              }
            return p;
          }
        else if (x->tavl_tag[0] == TAVL_THREAD)
          {
            struct tavl_node *p = x->tavl_link[0];
            if (p == NULL || p->tavl_link[1] != node)
              {
                while (y->tavl_tag[1] == TAVL_CHILD)
                  y = y->tavl_link[1];
                p = y->tavl_link[1];
              }
            return p;
          }
    }
  else
    return (struct tavl_node *) &tree->tavl_root;
}

/* Deletes from |tree| and returns an item matching |item|.
   Returns a null pointer if no matching item found. */
MARPA_TAVL_LINKAGE void *
marpa__tavl_delete (struct tavl_table *tree, const void *item)
{
  struct tavl_node *p; /* Traverses tree to find node to delete. */
  struct tavl_node *q; /* Parent of |p|. */
  int dir;             /* Index into |q->tavl_link[]| to get |p|. */
  int cmp;             /* Result of comparison between |item| and |p|. */

  assert (tree != NULL && item != NULL);

  if (tree->tavl_root == NULL)
    return NULL;

  q = (struct tavl_node *) &tree->tavl_root;
  p = tree->tavl_root;
  dir = 0;
  for (;;)
    {
      cmp = tree->tavl_compare (item, p->tavl_data, tree->tavl_param);
      if (cmp == 0)
        break;
      dir = cmp > 0;

      q = p;
      if (p->tavl_tag[dir] == TAVL_THREAD)
        return NULL;
      p = p->tavl_link[dir];
    }
  item = p->tavl_data;

  if (p->tavl_tag[1] == TAVL_THREAD)
    {
      if (p->tavl_tag[0] == TAVL_CHILD)
        {
          struct tavl_node *t = p->tavl_link[0];
          while (t->tavl_tag[1] == TAVL_CHILD)
            t = t->tavl_link[1];
          t->tavl_link[1] = p->tavl_link[1];
          q->tavl_link[dir] = p->tavl_link[0];
        }
      else
        {
          q->tavl_link[dir] = p->tavl_link[dir];
          if (q != (struct tavl_node *) &tree->tavl_root)
            q->tavl_tag[dir] = TAVL_THREAD;
        }
    }
  else
    {
      struct tavl_node *r = p->tavl_link[1];
      if (r->tavl_tag[0] == TAVL_THREAD)
        {
          r->tavl_link[0] = p->tavl_link[0];
          r->tavl_tag[0] = p->tavl_tag[0];
          if (r->tavl_tag[0] == TAVL_CHILD)
            {
              struct tavl_node *t = r->tavl_link[0];
              while (t->tavl_tag[1] == TAVL_CHILD)
                t = t->tavl_link[1];
              t->tavl_link[1] = r;
            }
          q->tavl_link[dir] = r;
          r->tavl_balance = p->tavl_balance;
          q = r;
          dir = 1;
        }
      else
        {
          struct tavl_node *s;

          for (;;)
            {
              s = r->tavl_link[0];
              if (s->tavl_tag[0] == TAVL_THREAD)
                break;

              r = s;
            }

          if (s->tavl_tag[1] == TAVL_CHILD)
            r->tavl_link[0] = s->tavl_link[1];
          else
            {
              r->tavl_link[0] = s;
              r->tavl_tag[0] = TAVL_THREAD;
            }

          s->tavl_link[0] = p->tavl_link[0];
          if (p->tavl_tag[0] == TAVL_CHILD)
            {
              struct tavl_node *t = p->tavl_link[0];
              while (t->tavl_tag[1] == TAVL_CHILD)
                t = t->tavl_link[1];
              t->tavl_link[1] = s;

              s->tavl_tag[0] = TAVL_CHILD;
            }

          s->tavl_link[1] = p->tavl_link[1];
          s->tavl_tag[1] = TAVL_CHILD;

          q->tavl_link[dir] = s;
          s->tavl_balance = p->tavl_balance;
          q = r;
          dir = 0;
        }
    }

  MY_AVL_FREE( p);

  while (q != (struct tavl_node *) &tree->tavl_root)
    {
      struct tavl_node *y = q;

      q = find_parent (tree, y);

      if (dir == 0)
        {
          dir = q->tavl_link[0] != y;
          y->tavl_balance++;
          if (y->tavl_balance == +1)
            break;
          else if (y->tavl_balance == +2)
            {
              struct tavl_node *x = y->tavl_link[1];

              assert (x != NULL);
              if (x->tavl_balance == -1)
                {
                  struct tavl_node *w;

                  assert (x->tavl_balance == -1);
                  w = x->tavl_link[0];
                  x->tavl_link[0] = w->tavl_link[1];
                  w->tavl_link[1] = x;
                  y->tavl_link[1] = w->tavl_link[0];
                  w->tavl_link[0] = y;
                  if (w->tavl_balance == +1)
                    x->tavl_balance = 0, y->tavl_balance = -1;
                  else if (w->tavl_balance == 0)
                    x->tavl_balance = y->tavl_balance = 0;
                  else /* |w->tavl_balance == -1| */
                    x->tavl_balance = +1, y->tavl_balance = 0;
                  w->tavl_balance = 0;
                  if (w->tavl_tag[0] == TAVL_THREAD)
                    {
                      y->tavl_tag[1] = TAVL_THREAD;
                      y->tavl_link[1] = w;
                      w->tavl_tag[0] = TAVL_CHILD;
                    }
                  if (w->tavl_tag[1] == TAVL_THREAD)
                    {
                      x->tavl_tag[0] = TAVL_THREAD;
                      x->tavl_link[0] = w;
                      w->tavl_tag[1] = TAVL_CHILD;
                    }
                  q->tavl_link[dir] = w;
                }
              else
                {
                  q->tavl_link[dir] = x;

                  if (x->tavl_balance == 0)
                    {
                      y->tavl_link[1] = x->tavl_link[0];
                      x->tavl_link[0] = y;
                      x->tavl_balance = -1;
                      y->tavl_balance = +1;
                      break;
                    }
                  else /* |x->tavl_balance == +1| */
                    {
                      if (x->tavl_tag[0] == TAVL_CHILD)
                        y->tavl_link[1] = x->tavl_link[0];
                      else
                        {
                          y->tavl_tag[1] = TAVL_THREAD;
                          x->tavl_tag[0] = TAVL_CHILD;
                        }
                      x->tavl_link[0] = y;
                      y->tavl_balance = x->tavl_balance = 0;
                    }
                }
            }
        }
      else
        {
          dir = q->tavl_link[0] != y;
          y->tavl_balance--;
          if (y->tavl_balance == -1)
            break;
          else if (y->tavl_balance == -2)
            {
              struct tavl_node *x = y->tavl_link[0];
              assert (x != NULL);
              if (x->tavl_balance == +1)
                {
                  struct tavl_node *w;

                  assert (x->tavl_balance == +1);
                  w = x->tavl_link[1];
                  x->tavl_link[1] = w->tavl_link[0];
                  w->tavl_link[0] = x;
                  y->tavl_link[0] = w->tavl_link[1];
                  w->tavl_link[1] = y;
                  if (w->tavl_balance == -1)
                    x->tavl_balance = 0, y->tavl_balance = +1;
                  else if (w->tavl_balance == 0)
                    x->tavl_balance = y->tavl_balance = 0;
                  else /* |w->tavl_balance == +1| */
                    x->tavl_balance = -1, y->tavl_balance = 0;
                  w->tavl_balance = 0;
                  if (w->tavl_tag[0] == TAVL_THREAD)
                    {
                      x->tavl_tag[1] = TAVL_THREAD;
                      x->tavl_link[1] = w;
                      w->tavl_tag[0] = TAVL_CHILD;
                    }
                  if (w->tavl_tag[1] == TAVL_THREAD)
                    {
                      y->tavl_tag[0] = TAVL_THREAD;
                      y->tavl_link[0] = w;
                      w->tavl_tag[1] = TAVL_CHILD;
                    }
                  q->tavl_link[dir] = w;
                }
              else
                {
                  q->tavl_link[dir] = x;

                  if (x->tavl_balance == 0)
                    {
                      y->tavl_link[0] = x->tavl_link[1];
                      x->tavl_link[1] = y;
                      x->tavl_balance = +1;
                      y->tavl_balance = -1;
                      break;
                    }
                  else /* |x->tavl_balance == -1| */
                    {
                      if (x->tavl_tag[1] == TAVL_CHILD)
                        y->tavl_link[0] = x->tavl_link[1];
                      else
                        {
                          y->tavl_tag[0] = TAVL_THREAD;
                          x->tavl_tag[1] = TAVL_CHILD;
                        }
                      x->tavl_link[1] = y;
                      y->tavl_balance = x->tavl_balance = 0;
                    }
                }
            }
        }
    }

  tree->tavl_count--;
  return (void *) item;
}

/* Initializes |trav| for use with |tree|
   and selects the null node. */
void
marpa__tavl_t_init (struct tavl_traverser *trav, struct tavl_table *tree)
{
  trav->tavl_table = tree;
  trav->tavl_node = NULL;
}

/* Initializes |trav| for |tree|.
   Returns data item in |tree| with the least value,
   or |NULL| if |tree| is empty. */
void *
marpa__tavl_t_first (struct tavl_traverser *trav, struct tavl_table *tree)
{
  assert (tree != NULL && trav != NULL);

  trav->tavl_table = tree;
  trav->tavl_node = tree->tavl_root;
  if (trav->tavl_node != NULL)
    {
      while (trav->tavl_node->tavl_tag[0] == TAVL_CHILD)
        trav->tavl_node = trav->tavl_node->tavl_link[0];
      return trav->tavl_node->tavl_data;
    }
  else
    return NULL;
}

/* Initializes |trav| for |tree|.
   Returns data item in |tree| with the greatest value,
   or |NULL| if |tree| is empty. */
void *
marpa__tavl_t_last (struct tavl_traverser *trav, struct tavl_table *tree)
{
  assert (tree != NULL && trav != NULL);

  trav->tavl_table = tree;
  trav->tavl_node = tree->tavl_root;
  if (trav->tavl_node != NULL)
    {
      while (trav->tavl_node->tavl_tag[1] == TAVL_CHILD)
        trav->tavl_node = trav->tavl_node->tavl_link[1];
      return trav->tavl_node->tavl_data;
    }
  else
    return NULL;
}

/* Searches for |item| in |tree|.
   If found, initializes |trav| to the item found and returns the item
   as well.
   If there is no matching item, initializes |trav| to the null item
   and returns |NULL|. */
void *
marpa__tavl_t_find (struct tavl_traverser *trav, struct tavl_table *tree, void *item)
{
  struct tavl_node *p;

  assert (trav != NULL && tree != NULL && item != NULL);

  trav->tavl_table = tree;
  trav->tavl_node = NULL;

  p = tree->tavl_root;
  if (p == NULL)
    return NULL;

  for (;;)
    {
      int cmp, dir;

      cmp = tree->tavl_compare (item, p->tavl_data, tree->tavl_param);
      if (cmp == 0)
        {
          trav->tavl_node = p;
          return p->tavl_data;
        }

      dir = cmp > 0;
      if (p->tavl_tag[dir] == TAVL_CHILD)
        p = p->tavl_link[dir];
      else
        return NULL;
    }
}

/* Attempts to insert |item| into |tree|.
   If |item| is inserted successfully, it is returned and |trav| is
   initialized to its location.
   If a duplicate is found, it is returned and |trav| is initialized to
   its location.  No replacement of the item occurs.
   If a memory allocation failure occurs, |NULL| is returned and |trav|
   is initialized to the null item. */
void *
marpa__tavl_t_insert (struct tavl_traverser *trav,
               struct tavl_table *tree, void *item)
{
  void **p;

  assert (trav != NULL && tree != NULL && item != NULL);

  p = marpa__tavl_probe (tree, item);
  if (p != NULL)
    {
      trav->tavl_table = tree;
      trav->tavl_node =
        ((struct tavl_node *)
         ((char *) p - offsetof (struct tavl_node, tavl_data)));
      return *p;
    }
  else
    {
      marpa__tavl_t_init (trav, tree);
      return NULL;
    }
}

/* Initializes |trav| to have the same current node as |src|. */
MARPA_TAVL_LINKAGE void *
marpa__tavl_t_copy (struct tavl_traverser *trav, const struct tavl_traverser *src)
{
  assert (trav != NULL && src != NULL);

  trav->tavl_table = src->tavl_table;
  trav->tavl_node = src->tavl_node;

  return trav->tavl_node != NULL ? trav->tavl_node->tavl_data : NULL;
}

/* Returns the next data item in inorder
   within the tree being traversed with |trav|,
   or if there are no more data items returns |NULL|. */
void *
marpa__tavl_t_next (struct tavl_traverser *trav)
{
  assert (trav != NULL);

  if (trav->tavl_node == NULL)
    return marpa__tavl_t_first (trav, trav->tavl_table);
  else if (trav->tavl_node->tavl_tag[1] == TAVL_THREAD)
    {
      trav->tavl_node = trav->tavl_node->tavl_link[1];
      return trav->tavl_node != NULL ? trav->tavl_node->tavl_data : NULL;
    }
  else
    {
      trav->tavl_node = trav->tavl_node->tavl_link[1];
      while (trav->tavl_node->tavl_tag[0] == TAVL_CHILD)
        trav->tavl_node = trav->tavl_node->tavl_link[0];
      return trav->tavl_node->tavl_data;
    }
}

/* Returns the previous data item in inorder
   within the tree being traversed with |trav|,
   or if there are no more data items returns |NULL|. */
void *
marpa__tavl_t_prev (struct tavl_traverser *trav)
{
  assert (trav != NULL);

  if (trav->tavl_node == NULL)
    return marpa__tavl_t_last (trav, trav->tavl_table);
  else if (trav->tavl_node->tavl_tag[0] == TAVL_THREAD)
    {
      trav->tavl_node = trav->tavl_node->tavl_link[0];
      return trav->tavl_node != NULL ? trav->tavl_node->tavl_data : NULL;
    }
  else
    {
      trav->tavl_node = trav->tavl_node->tavl_link[0];
      while (trav->tavl_node->tavl_tag[1] == TAVL_CHILD)
        trav->tavl_node = trav->tavl_node->tavl_link[1];
      return trav->tavl_node->tavl_data;
    }
}

/* Returns |trav|'s current item. */
void *
marpa__tavl_t_cur (struct tavl_traverser *trav)
{
  assert (trav != NULL);

  return trav->tavl_node != NULL ? trav->tavl_node->tavl_data : NULL;
}

/* Replaces the current item in |trav| by |new| and returns the item replaced.
   |trav| must not have the null item selected.
   The new item must not upset the ordering of the tree. */
void *
marpa__tavl_t_replace (struct tavl_traverser *trav, void *new)
{
  void *old;

  assert (trav != NULL && trav->tavl_node != NULL && new != NULL);
  old = trav->tavl_node->tavl_data;
  trav->tavl_node->tavl_data = new;
  return old;
}

/* Creates a new node as a child of |dst| on side |dir|.
   Copies data and |tavl_balance| from |src| into the new node,
   applying |copy()|, if non-null.
   Returns nonzero only if fully successful.
   Regardless of success, integrity of the tree structure is assured,
   though failure may leave a null pointer in a |tavl_data| member. */
static int
copy_node (struct tavl_table *tree,
           struct tavl_node *dst, int dir,
           const struct tavl_node *src, tavl_copy_func *copy)
{
  struct tavl_node *new =
    MY_AVL_MALLOC( sizeof *new);

  new->tavl_link[dir] = dst->tavl_link[dir];
  new->tavl_tag[dir] = TAVL_THREAD;
  new->tavl_link[!dir] = dst;
  new->tavl_tag[!dir] = TAVL_THREAD;
  dst->tavl_link[dir] = new;
  dst->tavl_tag[dir] = TAVL_CHILD;

  new->tavl_balance = src->tavl_balance;
  if (copy == NULL)
    new->tavl_data = src->tavl_data;
  else
    {
      new->tavl_data = copy (src->tavl_data, tree->tavl_param);
      if (new->tavl_data == NULL)
        return 0;
    }

  return 1;
}

/* Destroys |new| with |tavl_destroy (new, destroy)|,
   first initializing the right link in |new| that has
   not yet been initialized. */
static void
copy_error_recovery (struct tavl_node *p,
                     struct tavl_table *new, tavl_item_func *destroy)
{
  new->tavl_root = p;
  if (p != NULL)
    {
      while (p->tavl_tag[1] == TAVL_CHILD)
        p = p->tavl_link[1];
      p->tavl_link[1] = NULL;
    }
  marpa__tavl_destroy (new, destroy);
}

/* Copies |org| to a newly created tree, which is returned.
   If |copy != NULL|, each data item in |org| is first passed to |copy|,
   and the return values are inserted into the tree,
   with |NULL| return values taken as indications of failure.
   On failure, destroys the partially created new tree,
   applying |destroy|, if non-null, to each item in the new tree so far,
   and returns |NULL|.
   If |allocator != NULL|, it is used for allocation in the new tree.
   Otherwise, the same allocator used for |org| is used. */
struct tavl_table *
marpa__tavl_copy (const struct tavl_table *org, tavl_copy_func *copy,
          tavl_item_func *destroy)
{
  struct tavl_table *new;

  const struct tavl_node *p;
  struct tavl_node *q;
  struct tavl_node rp, rq;

  assert (org != NULL);
  new = marpa__tavl_create (org->tavl_compare, org->tavl_param);
  if (new == NULL)
    return NULL;

  new->tavl_count = org->tavl_count;
  if (new->tavl_count == 0)
    return new;

  p = &rp;
  rp.tavl_link[0] = org->tavl_root;
  rp.tavl_tag[0] = TAVL_CHILD;

  q = &rq;
  rq.tavl_link[0] = NULL;
  rq.tavl_tag[0] = TAVL_THREAD;

  for (;;)
    {
      if (p->tavl_tag[0] == TAVL_CHILD)
        {
          if (!copy_node (new, q, 0, p->tavl_link[0], copy))
            {
              copy_error_recovery (rq.tavl_link[0], new, destroy);
              return NULL;
            }

          p = p->tavl_link[0];
          q = q->tavl_link[0];
        }
      else
        {
          while (p->tavl_tag[1] == TAVL_THREAD)
            {
              p = p->tavl_link[1];
              if (p == NULL)
                {
                  q->tavl_link[1] = NULL;
                  new->tavl_root = rq.tavl_link[0];
                  return new;
                }

              q = q->tavl_link[1];
            }

          p = p->tavl_link[1];
          q = q->tavl_link[1];
        }

      if (p->tavl_tag[1] == TAVL_CHILD)
        if (!copy_node (new, q, 1, p->tavl_link[1], copy))
          {
            copy_error_recovery (rq.tavl_link[0], new, destroy);
            return NULL;
          }
    }
}

/* Frees storage allocated for |tree|.
   If |destroy != NULL|, applies it to each data item in inorder. */
MARPA_TAVL_LINKAGE void
marpa__tavl_destroy (struct tavl_table *tree, tavl_item_func *destroy)
{
  struct tavl_node *p; /* Current node. */
  struct tavl_node *n; /* Next node. */

  p = tree->tavl_root;
  if (p != NULL)
    while (p->tavl_tag[0] == TAVL_CHILD)
      p = p->tavl_link[0];

  while (p != NULL)
    {
      n = p->tavl_link[1];
      if (p->tavl_tag[1] == TAVL_CHILD)
        while (n->tavl_tag[0] == TAVL_CHILD)
          n = n->tavl_link[0];

      if (destroy != NULL && p->tavl_data != NULL)
        destroy (p->tavl_data, tree->tavl_param);
      MY_AVL_FREE( p);

      p = n;
    }

  MY_AVL_FREE( tree);
}

static void fatal(const char* message) {
    puts(message);
    abort();
}

/* Allocates |size| bytes of space using |malloc()|.
   Returns a null pointer if allocation fails. */
static void *
tavl_malloc (struct libavl_allocator *allocator UNUSED, size_t size UNUSED)
{
  fatal("Internal error: legacy TAVL malloc called");
  return NULL;
}

/* Frees |block|. */
static void tavl_free (struct libavl_allocator *allocator UNUSED, void *block UNUSED)
{
  fatal("Internal error: legacy TAVL free called");
}

/* Default memory allocator that uses |malloc()| and |free()|. */
/* Fail allocator -- should not be used and always fails. */
struct libavl_allocator default_allocator =
  {
    tavl_malloc,
    tavl_free
  };
struct libavl_allocator *marpa__tavl_allocator_default = &default_allocator;

#undef NDEBUG
#include <assert.h>

/* Asserts that |tavl_insert()| succeeds at inserting |item| into |table|. */
MARPA_TAVL_LINKAGE void
(marpa__tavl_assert_insert) (struct tavl_table *table, void *item)
{
  void **p = marpa__tavl_probe (table, item);
  assert (p != NULL && *p == item);
}

/* Asserts that |tavl_delete()| really removes |item| from |table|,
   and returns the removed item. */
MARPA_TAVL_LINKAGE void *
(marpa__tavl_assert_delete) (struct tavl_table *table, void *item)
{
  void *p = marpa__tavl_delete (table, item);
  assert (p != NULL);
  return p;
}


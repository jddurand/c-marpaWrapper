/* Copyright 2022 Jeffrey Kegler */

/* This is a modification of avl.h, from Ben Pfaff's libavl,
   which was under the LGPL 3.  Here is the copyright notice
   from that file:

   libavl - library for manipulation of binary trees.
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

#ifndef _MARPA_AVL_H__
#define _MARPA_AVL_H__ 1

#include <stddef.h>

/* Most use avl methods are insert and find.                    */
/* Because of the function pointer dereference of tavl_compare  */
/* Compilers cannot avoid jump calls, that have a visible cost. */
/* The following macros exhibit explicitly the comparison       */
/* function, so that compilers have a change to inline calls.   */

#define MARPA_AVL_FIND(avl_find_output, in_tree, in_item, cmp_func) do { \
    /* Input parameters are expressions. */                             \
    const MARPA_AVL_TREE avl_find_tree = (const MARPA_AVL_TREE) (in_tree); \
    const void *avl_find_item = (const void *) (in_item);               \
                                                                        \
    /* Implementation. */                                               \
    {                                                                   \
      short found = 0;                                                  \
      NODE avl_find_p;                                                  \
                                                                        \
      assert (avl_find_tree != NULL && avl_find_item != NULL);          \
      for (avl_find_p = avl_find_tree->avl_root; avl_find_p != NULL; )  \
        {                                                               \
          int cmp = cmp_func(avl_find_item, avl_find_p->avl_data, avl_find_tree->avl_param); \
                                                                        \
          if (cmp < 0)                                                  \
            avl_find_p = avl_find_p->avl_link[0];                       \
          else if (cmp > 0)                                             \
            avl_find_p = avl_find_p->avl_link[1];                       \
          else { /* |cmp == 0| */                                       \
            avl_find_output = avl_find_p->avl_data;                     \
            found = 1;                                                  \
            break;                                                      \
          }                                                             \
        }                                                               \
                                                                        \
      if (! found) {                                                    \
        avl_find_output = NULL;                                         \
      }                                                                 \
    }                                                                   \
 } while (0)

#define MARPA_AVL_PROBE(avl_probe_output, in_tree, in_item, cmp_func) do { \
    /* Input parameters are expressions. */                             \
    MARPA_AVL_TREE avl_probe_tree = (MARPA_AVL_TREE) (in_tree);         \
    void *avl_probe_item = (void *) (in_item);                          \
                                                                        \
    /* Implementation. */                                               \
    {                                                                   \
      NODE y, z; /* Top node to update balance factor, and parent */    \
      NODE p, q; /* Iterator, and parent. */                            \
      NODE n;     /* Newly inserted node. */                            \
      NODE w;     /* New root of rebalanced subtree. */                 \
      int dir;                /* Direction to descend. */               \
                                                                        \
      unsigned char da[MARPA_AVL_MAX_HEIGHT]; /* Cached comparison results. */ \
      int k = 0;              /* Number of cached results. */           \
      short found = 0;                                                  \
                                                                        \
      assert (avl_probe_tree != NULL && avl_probe_item != NULL);        \
                                                                        \
      z = (NODE) &avl_probe_tree->avl_root;                             \
      y = avl_probe_tree->avl_root;                                     \
      dir = 0;                                                          \
      for (q = z, p = y; p != NULL; q = p, p = p->avl_link[dir])        \
        {                                                               \
          int cmp = cmp_func(avl_probe_item, p->avl_data, avl_probe_tree->avl_param); \
          if (cmp == 0) {                                               \
            avl_probe_output = &p->avl_data;                            \
            found = 1;                                                  \
            break;                                                      \
          }                                                             \
                                                                        \
          if (p->avl_balance != 0)                                      \
            z = q, y = p, k = 0;                                        \
          dir = cmp > 0;                                                \
          da[k++] = (unsigned char)dir;                                 \
        }                                                               \
                                                                        \
      if (! found) {                                                    \
        n = q->avl_link[dir] = marpa_obs_new (avl_probe_tree->avl_obstack, struct marpa_avl_node, 1); \
                                                                        \
        avl_probe_tree->avl_count++;                                    \
        n->avl_data = avl_probe_item;                                   \
        n->avl_link[0] = n->avl_link[1] = NULL;                         \
        n->avl_balance = 0;                                             \
        if (y == NULL) {                                                \
          avl_probe_output = &n->avl_data;                              \
        } else {                                                        \
          for (p = y, k = 0; p != n; p = p->avl_link[da[k]], k++)       \
            if (da[k] == 0)                                             \
              p->avl_balance--;                                         \
            else                                                        \
              p->avl_balance++;                                         \
                                                                        \
          if (y->avl_balance == -2)                                     \
            {                                                           \
              NODE x = y->avl_link[0];                                  \
              if (x->avl_balance == -1)                                 \
                {                                                       \
                  w = x;                                                \
                  y->avl_link[0] = x->avl_link[1];                      \
                  x->avl_link[1] = y;                                   \
                  x->avl_balance = y->avl_balance = 0;                  \
                }                                                       \
              else                                                      \
                {                                                       \
                  assert (x->avl_balance == +1);                        \
                  w = x->avl_link[1];                                   \
                  x->avl_link[1] = w->avl_link[0];                      \
                  w->avl_link[0] = x;                                   \
                  y->avl_link[0] = w->avl_link[1];                      \
                  w->avl_link[1] = y;                                   \
                  if (w->avl_balance == -1)                             \
                    x->avl_balance = 0, y->avl_balance = +1;            \
                  else if (w->avl_balance == 0)                         \
                    x->avl_balance = y->avl_balance = 0;                \
                  else /* |w->avl_balance == +1| */                     \
                    x->avl_balance = -1, y->avl_balance = 0;            \
                  w->avl_balance = 0;                                   \
                }                                                       \
            }                                                           \
          else if (y->avl_balance == +2)                                \
            {                                                           \
              NODE x = y->avl_link[1];                                  \
              if (x->avl_balance == +1)                                 \
                {                                                       \
                  w = x;                                                \
                  y->avl_link[1] = x->avl_link[0];                      \
                  x->avl_link[0] = y;                                   \
                  x->avl_balance = y->avl_balance = 0;                  \
                }                                                       \
              else                                                      \
                {                                                       \
                  assert (x->avl_balance == -1);                        \
                  w = x->avl_link[0];                                   \
                  x->avl_link[0] = w->avl_link[1];                      \
                  w->avl_link[1] = x;                                   \
                  y->avl_link[1] = w->avl_link[0];                      \
                  w->avl_link[0] = y;                                   \
                  if (w->avl_balance == +1)                             \
                    x->avl_balance = 0, y->avl_balance = -1;            \
                  else if (w->avl_balance == 0)                         \
                    x->avl_balance = y->avl_balance = 0;                \
                  else /* |w->avl_balance == -1| */                     \
                    x->avl_balance = +1, y->avl_balance = 0;            \
                  w->avl_balance = 0;                                   \
                }                                                       \
            }                                                           \
          else {                                                        \
            avl_probe_output =  &n->avl_data;                           \
            found = 1;                                                  \
          }                                                             \
          if (! found) {                                                \
            z->avl_link[y != z->avl_link[0]] = w;                       \
                                                                        \
            avl_probe_tree->avl_generation++;                           \
            avl_probe_output = &n->avl_data;                            \
          }                                                             \
        }                                                               \
      }                                                                 \
    }                                                                   \
  } while (0)

#define MARPA_AVL_INSERT(avl_insert_output, in_table, in_item, cmp_func) do { \
    /* Input parameters are expressions. */                             \
    MARPA_AVL_TREE avl_insert_table = (MARPA_AVL_TREE) (in_table);      \
    void *avl_insert_item = (void *) (in_item);                         \
                                                                        \
    /* Implementation. */                                               \
    {                                                                   \
      void **avl_insert_p;                                              \
      MARPA_AVL_PROBE(avl_insert_p, avl_insert_table, avl_insert_item, cmp_func); \
      avl_insert_output = avl_insert_p == NULL || *avl_insert_p == avl_insert_item ? NULL : *avl_insert_p; \
    }                                                                   \
  } while (0)

/* The linkage macros (MARPA_.*LINKAGE) are useful for specifying
   alternative linkage, usually 'static'.  The intended use case is
   including the Marpa source in a single file, and redefining
   the linkage macros on the command line:

-DMARPA_LINKAGE=static -DMARPA_AVL_LINKAGE=static -DMARPA_TAVL_LINKAGE=static -DMARPA_OBS_LINKAGE=static

   However, it is important to note that any redefinition of the linkaage
   macros is currently experimental, and therefore unsupported.
*/
#ifndef MARPA_AVL_LINKAGE
#  define MARPA_AVL_LINKAGE /* Default linkage */
#endif

/* Function types. */
typedef int marpa_avl_comparison_func (const void *avl_a, const void *avl_b,
                                 void *avl_param);
typedef void marpa_avl_item_func (void *avl_item, void *avl_param);
typedef void *marpa_avl_copy_func (void *avl_item, void *avl_param);

/* An AVL tree node. */
struct marpa_avl_node
  {
    struct marpa_avl_node *avl_link[2];  /* Subtrees. */
    void *avl_data;                /* Pointer to data. */
    signed char avl_balance;       /* Balance factor. */
  };

/* Tree data structure. */
struct marpa_avl_table
  {
    struct marpa_avl_node *avl_root;          /* Tree's root. */
    marpa_avl_comparison_func *avl_compare;   /* Comparison function. */
    void *avl_param;                    /* Extra argument to |avl_compare|. */
    struct marpa_obstack *avl_obstack;
    size_t avl_count;                   /* Number of items in tree. */
    unsigned long avl_generation;       /* Generation number. */
  };
typedef struct marpa_avl_table* MARPA_AVL_TREE;

/* Maximum AVL tree height. */
#ifndef MARPA_AVL_MAX_HEIGHT
#define MARPA_AVL_MAX_HEIGHT 92
#endif

/* AVL traverser structure. */
struct marpa_avl_traverser
  {
    MARPA_AVL_TREE avl_table;        /* Tree being traversed. */
    struct marpa_avl_node *avl_node;          /* Current node in tree. */
    struct marpa_avl_node *avl_stack[MARPA_AVL_MAX_HEIGHT];
                                        /* All the nodes above |avl_node|. */
    size_t avl_height;                  /* Number of nodes in |avl_parent|. */
    unsigned long avl_generation;       /* Generation number. */
  };
typedef struct marpa_avl_traverser* MARPA_AVL_TRAV;

#define MARPA_TREE_OF_AVL_TRAV(trav) ((trav)->avl_table)
#define MARPA_DATA_OF_AVL_TRAV(trav) ((trav)->avl_node ? (trav)->avl_node->avl_data : NULL)
#define MARPA_AVL_OBSTACK(table) ((table)->avl_obstack)

/* Table functions. */
MARPA_AVL_LINKAGE MARPA_AVL_TREE _marpa_avl_create (marpa_avl_comparison_func *, void *);
MARPA_AVL_LINKAGE MARPA_AVL_TREE _marpa_avl_copy (const MARPA_AVL_TREE , marpa_avl_copy_func *,
                            marpa_avl_item_func *, int alignment);
MARPA_AVL_LINKAGE void _marpa_avl_destroy (MARPA_AVL_TREE );
MARPA_AVL_LINKAGE void **_marpa_avl_probe (MARPA_AVL_TREE , void *);
MARPA_AVL_LINKAGE void *_marpa_avl_insert (MARPA_AVL_TREE , void *);
MARPA_AVL_LINKAGE void *_marpa_avl_replace (MARPA_AVL_TREE , void *);
MARPA_AVL_LINKAGE void *_marpa_avl_find (const MARPA_AVL_TREE , const void *);
MARPA_AVL_LINKAGE void *_marpa_avl_at_or_after (const MARPA_AVL_TREE , const void *);

#define marpa_avl_count(table) ((size_t) (table)->avl_count)

/* Table traverser functions. */
MARPA_AVL_LINKAGE MARPA_AVL_TRAV _marpa_avl_t_init (MARPA_AVL_TREE );
MARPA_AVL_LINKAGE MARPA_AVL_TRAV _marpa_avl_t_reset (MARPA_AVL_TRAV );
MARPA_AVL_LINKAGE void *_marpa_avl_t_first (MARPA_AVL_TRAV );
MARPA_AVL_LINKAGE void *_marpa_avl_t_last ( MARPA_AVL_TRAV );
MARPA_AVL_LINKAGE void *_marpa_avl_t_find ( MARPA_AVL_TRAV , void *);
MARPA_AVL_LINKAGE void *_marpa_avl_t_copy (struct marpa_avl_traverser *, const struct marpa_avl_traverser *);
MARPA_AVL_LINKAGE void *_marpa_avl_t_next (MARPA_AVL_TRAV);
MARPA_AVL_LINKAGE void *_marpa_avl_t_prev (MARPA_AVL_TRAV);
MARPA_AVL_LINKAGE void *_marpa_avl_t_cur (MARPA_AVL_TRAV);
MARPA_AVL_LINKAGE void *_marpa_avl_t_insert (MARPA_AVL_TRAV, void *);
MARPA_AVL_LINKAGE void *_marpa_avl_t_replace (MARPA_AVL_TRAV, void *);
MARPA_AVL_LINKAGE void *_marpa_avl_t_at_or_after (MARPA_AVL_TRAV, void*);

#endif /* marpa_avl.h */

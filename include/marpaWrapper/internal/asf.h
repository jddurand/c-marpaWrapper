#ifndef MARPAWRAPPER_INTERNAL_ASF_H
#define MARPAWRAPPER_INTERNAL_ASF_H

#include <stddef.h>
#include <genericStack.h>
#include <genericHash.h>
#include <genericSparseArray.h>
#include "marpaWrapper/asf.h"
#include "marpa.h"

/* Is the "registered" attribute of marpaWrapperAsfGlade_t really needed ? I don't believe */
#ifndef MARPAWRAPPERASF_USE_REGISTERED_FLAG
#define MARPAWRAPPERASF_USE_REGISTERED_FLAG 0
#endif

typedef struct marpaWrapperAsfIdset {
  int    idi;
  int    counti;
  int   *idip;
} marpaWrapperAsfIdset_t;

typedef marpaWrapperAsfIdset_t marpaWrapperAsfNidset_t;
typedef marpaWrapperAsfIdset_t marpaWrapperAsfPowerset_t;

typedef enum marpaWrapperAsfIdsete {
  MARPAWRAPPERASFIDSET_NIDSET = 0,
  MARPAWRAPPERASFIDSET_POWERSET,
  _MARPAWRAPPERASFIDSET_IDSETE_MAX
} marpaWrapperAsfIdsete_t;

typedef struct marpaWrapperAsfGlade {
  int             idi;
  genericStack_t *symchesStackp;
  short           visitedb;
#if MARPAWRAPPERASF_USE_REGISTERED_FLAG > 0
  short           registeredb;
#endif
} marpaWrapperAsfGlade_t;

typedef int *andNodeStack_t;

typedef struct marpaWrapperAsfOrNode {
  int     nAndNodei;
  int    *andNodep;
} marpaWrapperAsfOrNode_t;

typedef struct marpaWrapperAsfNook {
  int   orNodeIdi;
  int   firstChoicei;
  int   lastChoicei;
  int   parentOrNodeIdi;
  short isCauseb;
  short isPredecessorb;
  short causeIsExpandedb;
  short predecessorIsExpandedb;
} marpaWrapperAsfNook_t;

typedef struct marpaWrapperAsfSourceData {
  int sortIxi;
  int sourceNidi;
} marpaWrapperAsfSourceData_t;

struct marpaWrapperAsf {
  marpaWrapperRecognizer_t    *marpaWrapperRecognizerp;
  marpaWrapperAsfOption_t      marpaWrapperAsfOption;
  Marpa_Bocage                 marpaBocagep;
  Marpa_Order                  marpaOrderp;
  int                          recursionLeveli;
  int                          nParsesi;

  /* Memoization */
  genericStack_t              *orNodeStackp;
  genericHash_t               *intsetHashp;
  genericSparseArray_t        *nidsetSparseArrayp;
  genericSparseArray_t        *powersetSparseArrayp;
  genericStack_t              *gladeStackp;

  /* Memoization of choices */
  int                         nextIntseti;

  /* Traverser callback */
  traverserCallback_t         traverserCallbackp;
  void                       *userDatavp;

  /* For optimizations, internal generic stacks of methods */
  /* that do not recurse are setted once */
  genericStack_t             *worklistStackp;

  /* For optimization of intset memoization */
  int                        *intsetidp;
  int                         intsetcounti;
  int                        *causeNidsp;
  int                         causeNidsi;

  /* For optimization of _marpaWrapperAsf_glade_obtainp() */
  genericStack_t             *gladeObtainTmpStackp;

  /* For optimization of _marpaWrapperAsf_and_nodes_to_cause_nidsp() */
  genericHash_t              *causesHashp;
};

typedef struct marpaWrapperAsfChoicePoint {
  genericStack_t         *factoringStackp;
  genericSparseArray_t   *orNodeInUseSparseArrayp;
} marpaWrapperAsfChoicePoint_t;

struct marpaWrapperAsfTraverser {
  marpaWrapperAsf_t      *marpaWrapperAsfp;
  genericSparseArray_t   *valueSparseArrayp;
  marpaWrapperAsfGlade_t *gladep;
  int                     symchIxi;
  int                     factoringIxi;
};

/* Internal structure used for valuation using the ASF */
typedef struct marpaWrapperAsfPrunedValueContext {
  /* Copy of the marpaWrapperAsf_valueb() parameters */
  void                                 *userDatavp;
  marpaWrapperAsfOkRuleCallback_t       okRuleCallbackp;
  marpaWrapperAsfOkSymbolCallback_t     okSymbolCallbackp;
  marpaWrapperAsfOkNullingCallback_t    okNullingCallbackp;
  marpaWrapperValueRuleCallback_t       valueRuleCallbackp;
  marpaWrapperValueSymbolCallback_t     valueSymbolCallbackp;
  marpaWrapperValueNullingCallback_t    valueNullingCallbackp;
  /* Parent rule ID stack */
  genericStack_t                       *parentRuleiStackp;
  /* Current wanted indice in the output stack */
  int                                   wantedOutputStacki;
  /* For logging, keep track of recursivity level */
  int                                   leveli;
} marpaWrapperAsfPrunedValueContext_t;

#endif /* MARPAWRAPPER_INTERNAL_ASF_H */

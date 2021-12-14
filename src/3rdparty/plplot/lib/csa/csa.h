//--------------------------------------------------------------------------
//
// File:           csa.h
//
// Created:        16/10/2002
//
// Author:         Pavel Sakov
//                 CSIRO Marine Research
//
// Purpose:        A header for csa library (2D data approximation with
//                 bivariate C1 cubic spline)
//
// Revisions:      None
//
//--------------------------------------------------------------------------

#if !defined ( _CSA_H )
#define _CSA_H

// include header file for dll definitions
#include "csadll.h"

#if _MSC_VER > 1309
  #define hypot    _hypot
#endif

#if !defined ( _POINT_STRUCT )
#define _POINT_STRUCT
typedef struct
{
    double x;
    double y;
    double z;
} point;
#endif

extern int       csa_verbose;
extern const char* csa_version;

struct csa;
typedef struct csa   csa;

CSADLLIMPEXP
csa* csa_create( void );
CSADLLIMPEXP
void csa_destroy( csa* a );
CSADLLIMPEXP
void csa_addpoints( csa* a, int n, point points[] );
CSADLLIMPEXP
void csa_calculatespline( csa* a );
CSADLLIMPEXP
void csa_approximate_point( csa* a, point* p );
CSADLLIMPEXP
void csa_approximate_points( csa* a, int n, point* points );

CSADLLIMPEXP
void csa_setnpmin( csa* a, int npmin );
CSADLLIMPEXP
void csa_setnpmax( csa* a, int npmax );
CSADLLIMPEXP
void csa_setk( csa* a, int k );
CSADLLIMPEXP
void csa_setnpps( csa* a, double npps );

#endif

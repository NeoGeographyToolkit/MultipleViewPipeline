#include <math.h>
#include <limits.h>
#include <float.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>

#define ZERO ((double) 0)
#define ONE  ((double) 1)
#define TWO  ((double) 2)
#define MAX(a,b) (((a) > (b)) ? (a) : (b))
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

typedef struct asa_com_struct /* common variables */
{
    /* parameters computed by the code */
    double          *lo ; /* lower bounds */
    double          *hi ; /* lower bounds */
    double           *x ; /* current estimate for solution */
    double           *d ; /* current search direction */
    double           *g ; /* current gradient */
    double       *xtemp ; /* x + alpha*d */
    double       *gtemp ; /* gradient at x + alpha*d */
    double          *pg ; /* projected gradient */
    double *lastfvalues ; /* previous function values */
    double      minstep ; /* smallest step to reach a bound */
    double      maxstep ; /* step which makes all bounds active */
    double        bdist ; /* closest distance to bounds */
    int         minflag ; /* T (minstep correct), F (minstep is lower bound) */
    int           nfree ; /* number of free variables */
    int          *ifree ; /* indices of free variables */
    int               n ; /* problem dimension, saved for reference */
    int              n5 ; /* n % 5 */
    INT              nf ; /* total number of function evaluations */
    INT              ng ; /* total number of gradient evaluations */
    INT         cbbiter ; /* total number of cbb iterations */
    INT          cgiter ; /* total number of cg iterations */
    INT         cbbfunc ; /* total cbb function evaluations */
    INT         cbbgrad ; /* total cbb gradient evaluations */
    INT          cgfunc ; /* total cg function evaluations */
    INT          cggrad ; /* total cg gradient evaluations */
    INT         cgmaxit ; /* maximum number of iterations in one pass of cg */
    INT         pgmaxit ; /* max iterations in one pass of gradient projection*/
    INT       pgmaxfunc ; /* max function evals/pass of gradient projection */

    double        alpha ; /* stepsize along search direction */
    double            f ; /* function value for step alpha */
    double      f_debug ; /* function value at time of debug failure */
    double           df ; /* function derivative for step alpha */
    double        fpert ; /* perturbation is eps*Ck if PertRule is T */
    double          eps ; /* current value of eps */
    double           f0 ; /* old function value */
    double           Ck ; /* average cost as given by the rule:
                             Qk = Qdecay*Qk + 1, Ck += (fabs (f) - Ck)/Qk */
    double     wolfe_hi ; /* upper bound for slope in Wolfe test */
    double     wolfe_lo ; /* lower bound for slope in Wolfe test */
    double    awolfe_hi ; /* upper bound for slope, approximate Wolfe test */
    int          QuadOK ; /* T (quadratic step successful) */
    int          AWolfe ; /* F (use Wolfe line search)
                             T (use approximate Wolfe line search)
                             do not change user's AWolfe, this value can be
                             changed based on AWolfeFac */
    int       DimReduce ;  /*T (compressed problem, nfree < n)
                             F (work in full space, nfree = n)*/
    int         AArmijo ; /* F (use nonmonotone Armijo line search)
                             T (use approximate nonmonotone Armijo line search)
                             do not change user's AArmijo, this value can be
                             changed based on AArmijoFac */
    double          sts ; /* ||s||^2 */
    double          gtd ; /* g'd */
    double          sty ; /* s'y */
    double      pert_lo ; /* perturbation of lower bounds */
    double      pert_hi ; /* perturbation of upper bounds */
    double         tau1 ; /* if ginorm < tau1*pgnorm, continue gp steps  */
    double         tau2 ; /* ginorm < tau2*pgnorm =>
                             subproblem solved in cgdescent */
    double pgnorm_start ; /* ||Proj (x_0 - g_0) - x_0||_infty */
    double          tol ; /* convergence tolerance */
    double       pgnorm ; /* project gradient norm */
    double       ginorm ; /* norm of inactive gradient components */
    asacg_parm  *cgParm ; /* cg user parameters */
    asa_parm   *asaParm ; /* asa user parameters */
    asa_objective *user ; /* information passed to user when function or
                             gradient must be evaluated */
    double        (*value) (asa_objective *) ; /* evaluate objective function */
    void           (*grad) (asa_objective *) ; /* evaluate objective gradient */
    double      (*valgrad) (asa_objective *) ; /* function & gradient if given*/
} asa_com ;

/* prototypes */

int asa_descent /*  return:
                      -5 (ginorm < tau2*pgnorm without hitting boundary)
                      -4 (ginorm >=tau2*pgnorm, many x components hit boundary)
                      -3 (ginorm >=tau2*pgnorm, one x component hits boundary)
                      -2 (function value became nan)
                      -1 (starting function value is nan)
                       0 (convergence tolerance satisfied)
                       1 (change in func <= feps*|f|)
                       2 (total iterations exceeded maxit)
                       3 (slope always negative in line search)
                       4 (number secant iterations exceed nsecant)
                       5 (search direction not a descent direction)
                       6 (line search fails in initial interval)
                       7 (line search fails during bisection)
                       8 (line search fails during interval update)
                       9 (debugger is on and the function value increases)*/
(
    asa_com *Com
) ;

int asa_Wolfe
(
    double       alpha , /* stepsize */
    double           f , /* function value associated with stepsize alpha */
    double        dphi , /* derivative value associated with stepsize alpha */
    asa_com        *Com  /* cg com */
) ;

int asa_tol
(
    double      pgnorm, /* projected gradient sup-norm */
    asa_com       *Com
) ;

double asa_dot /* dot product between x and y */
(
    double *x , /* first vector */
    double *y , /* second vector */
    int     n   /* length of vectors */
) ;

void asa_copy  /* Copy vector x into vector y */
(
    double *y,
    double *x,
    int     n
) ;

void asa_saxpy  /* Compute z = y + ax */
(
    double *z,
    double *y,
    double *x,
    double  a,
    int     n
) ;

double asa_max /* Return max {fabs (x [j]) : 1 <= j < n}*/
(
    double *x,
    int     n
) ;

void asa_step /* Compute xtemp = x + alpha d */
(
    double *xtemp , /*output vector */
    double     *x , /* initial vector */
    double     *d , /* search direction */
    double  alpha , /* stepsize */
    INT         n   /* length of the vectors */
) ;

int asa_line
(
    double       dphi0, /* function derivative at starting point (alpha = 0) */
    asa_com       *Com  /* cg com structure */
) ;

int asa_lineW
(
    double       dphi0 , /* function derivative at starting point (alpha = 0) */
    asa_com       *Com   /* cg com structure */
) ;

int asa_update
(
    double          *a , /* left side of bracketing interval */
    double      *dphia , /* derivative at a */
    double          *b , /* right side of bracketing interval */
    double      *dphib , /* derivative at b */
    double      *alpha , /* trial step (between a and b) */
    double        *phi , /* function value at alpha (returned) */
    double       *dphi , /* function derivative at alpha (returned) */
    asa_com       *Com   /* cg com structure */
) ;

int asa_updateW
(
    double          *a , /* left side of bracketing interval */
    double      *dpsia , /* derivative at a */
    double          *b , /* right side of bracketing interval */
    double      *dpsib , /* derivative at b */
    double      *alpha , /* trial step (between a and b) */
    double        *phi , /* function value at alpha (returned) */
    double       *dphi , /* derivative of phi at alpha (returned) */
    double       *dpsi , /* derivative of psi at alpha (returned) */
    asa_com       *Com   /* cg com structure */
) ;

void asa_project
(
    double  *xnew,
    double     *x,
    double     *d,
    double  alpha,
    asa_com  *Com   /* cg com structure */
) ;

void asa_maxstep
(
    double       *x, /* current iterate */
    double       *d, /* direction */
    asa_com    *Com
) ;

int asa_grad_proj /*return:
                      -1 (proceed to cg routine)
                       0 (convergence tolerance satisfied)
                      10 (number of iterations exceeds limit)
                      11 (line search fails) */
(
    asa_com *Com
) ;

double asa_f
(
    double    *x,
    asa_com *Com
) ;

void asa_g
(
    double    *g,
    double    *x,
    asa_com *Com
) ;

double asa_fg
(
    double    *g,
    double    *x,
    asa_com *Com
) ;

int asa_identify
(
   double     *x,
   double     *g,
   double pgnorm,
   asa_com  *Com
) ;

double asa_init_bbstep
(
    asa_com *Com
) ;

void asa_expandx
(
    double    *x,
    asa_com *Com
) ;

void asa_shrinkx
(
    double    *x,
    asa_com *Com
) ;

void asa_shrinkxg
(
    double    *x,
    double    *g,
    asa_com *Com
) ;

void asa_expand_all
(
    asa_com *Com
) ;

void asa_shrink_all
(
    asa_com *Com
) ;

void asa_printcgParms
(
    asacg_parm  *Parm
) ;

void asa_printParms
(
    asa_parm  *Parm
) ;

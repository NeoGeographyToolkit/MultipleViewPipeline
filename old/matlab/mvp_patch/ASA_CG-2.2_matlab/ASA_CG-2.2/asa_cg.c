/* =========================================================================
   ============================== ASA_CG ===================================
   =========================================================================
       ________________________________________________________________
      | A Conjugate Gradient (cg_descent) based Active Set Algorithm   |
      |                                                                |
      |                  Version 1.0 (May 18, 2008)                    |
      |                  Version 1.1 (June 29, 2008)                   |
      |                  Version 1.2 (September 10, 2008)              |
      |                  Version 1.3 (September 25, 2009)              |
      |                  Version 2.0 (March 30, 2011)                  |
      |                  Version 2.1 (April 4, 2011)                   |
      |                  Version 2.2 (April 14, 2011)                  |
      |                                                                |
      |        William W. Hager    and      Hongchao Zhang             |
      |        hager@math.ufl.edu         hzhang@math.ufl.edu          |
      |  Department of Mathematics      Department of Mathematics      |
      |     University of Florida       Louisiana State University     |
      |  Gainesville, Florida 32611      Baton Rouge, Louisiana        |
      |     352-392-0281 x 244                                         |
      |                                                                |
      |      Copyright by William W. Hager and Hongchao Zhang          |
      |                                                                |
      |          http://www.math.ufl.edu/~hager/papers/CG              |
      |________________________________________________________________|
       ________________________________________________________________
      |This program is free software; you can redistribute it and/or   |
      |modify it under the terms of the GNU General Public License as  |
      |published by the Free Software Foundation; either version 2 of  |
      |the License, or (at your option) any later version.             |
      |This program is distributed in the hope that it will be useful, |
      |but WITHOUT ANY WARRANTY; without even the implied warranty of  |
      |MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   |
      |GNU General Public License for more details.                    |
      |                                                                |
      |You should have received a copy of the GNU General Public       |
      |License along with this program; if not, write to the Free      |
      |Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, |
      |MA  02110-1301  USA                                             |
      |________________________________________________________________|*/

#include "asa_user.h"
#include "asa_cg.h"
int asa_cg /*  return:
                      -2 (function value became nan in cg)
                      -1 (starting function value is nan in cg)
                       0 (convergence tolerance satisfied)
                       1 (change in func <= feps*|f| in cg)
                       2 (cg iterations in all passes or
                          in one pass exceeded their limit)
                       3 (slope always negative in line search in cg)
                       4 (number secant iterations exceed nsecant in cg)
                       5 (search direction not a descent direction in cg)
                       6 (line search fails in initial interval in cg)
                       7 (line search fails during bisection in cg)
                       8 (line search fails during interval update in cg)
                       9 (debugger is on and the function value increases in cg)
                      10 (out of memory)
                      11 (cbb iterations in all passes or
                          in one pass exceeded their limit)
                      12 (line search failed in cbb iteration)
                      13 (search direction in cbb is not descent direction)
                      14 (function value became nan in cbb) */
(
    double            *x, /* input: starting guess, output: the solution */
    double           *lo, /* lower bounds */
    double           *hi, /* upper bounds */
    INT                n, /* problem dimension */
    asa_stat       *Stat, /* structure with statistics (can be NULL) */
    asacg_parm    *CParm, /* user parameters, NULL = use default parameters */
    asa_parm      *AParm, /* user parameters, NULL = use default parameters */
    double      grad_tol, /* |Proj (x_k - g_k) - x_k|_inf <= grad_tol */
    double   (*value) (asa_objective *), /* evaluate objective function */
    void      (*grad) (asa_objective *), /* evaluate objective gradient */
    double (*valgrad) (asa_objective *), /* function and gradient
                                            NULL = use value & grad routines */
    double        *Work  /* either work array of size 7n + memory (m) or NULL */
)
{
    int gp, ident, j, nfree, status, *ifree ;
    INT cbb_totit, cg_totit ;
    double alpha, gj, pert_lo, pert_hi, t, tl, th, gnorm, ginorm, pgnorm, xnorm,
           xj, xg, xp, *work, *d, *g, *xtemp, *gtemp, *pg ;
    asacg_parm *cgParm, cgParmStruc ;
    asa_parm *asaParm, asaParmStruc ;
    asa_com Com ;
    asa_objective Objective ;

/* initialize the parameters */

    if ( CParm == NULL )
    {
        cgParm = &cgParmStruc ;
        asa_cg_default (cgParm) ;
    }
    else cgParm = CParm ;
    if ( cgParm->PrintParms ) asa_printcgParms (cgParm) ;

    if ( AParm == NULL )
    {
        asaParm = &asaParmStruc ;
        asa_default (asaParm) ;
    }
    else asaParm = AParm ;
    if ( asaParm->PrintParms ) asa_printParms (asaParm) ;

    /* abort after maxit iterations of cbb in one pass */
    if ( asaParm->maxit_fac == INF ) Com.pgmaxit = INT_INF ;
    else Com.pgmaxit = (INT) (((double) n)*asaParm->maxit_fac) ;

    /* abort after totit iterations of cbb in all passes */
    if ( asaParm->totit_fac == INF ) cbb_totit = INT_INF ;
    else cbb_totit = (INT) (((double) n)*asaParm->totit_fac) ;

    /* abort after maxfunc function evaluation in one pass of cbb */
    if ( asaParm->maxfunc_fac == INF ) Com.pgmaxfunc = INT_INF ;
    else Com.pgmaxfunc = (INT) (((double) n)*asaParm->maxfunc_fac) ;

    /* abort after totit iterations of cg in all passes */
    if ( cgParm->totit_fac == INF ) cg_totit = INT_INF ;
    else cg_totit = (INT) (((double) n)*cgParm->totit_fac) ;

    Com.eps = cgParm->eps ;

    pert_lo = asaParm->pert_lo ;
    pert_hi = asaParm->pert_hi ;
    Com.user = &Objective ;
    Objective.n = n ;
    Com.tau1 = asaParm->tau1 ;
    Com.tau2 = asaParm->tau2 ;

    Com.cgParm = cgParm ;
    Com.asaParm = asaParm ;
    Com.x = x ;
    Com.n = n ;             /* problem dimension */
    Com.n5 = n % 5 ;
    Com.nf = (INT) 0 ;      /* number of function evaluations */
    Com.ng = (INT) 0 ;      /* number of gradient evaluations */
    Com.cbbiter = (INT) 0 ; /* number of cbb iterations evaluations */
    Com.cgiter = (INT) 0 ;  /* number of cg iterations */
    Com.AWolfe = cgParm->AWolfe ; /* do not touch user's AWolfe */
    Com.AArmijo = asaParm->AArmijo ; /* do not touch user's AArmijo */
    Com.value = value ;
    Com.grad = grad ;
    Com.valgrad = valgrad ;
    Com.DimReduce = FALSE ;
    ifree = Com.ifree = malloc (n*sizeof (int)) ;

    if ( Work == NULL ) work = malloc ((5*n+asaParm->m)*sizeof (double)) ;
    else                work = Work ;
    if ( work == NULL )
    {
        printf ("Insufficient memory for specified problem dimension %e\n",
                 (double) n) ;
        status = 10 ;
        return (status) ;
    }
    d = Com.d = work ;
    g = Com.g = d+n ;
    xtemp = Com.xtemp = g+n ;
    gtemp = Com.gtemp = xtemp+n ;
    pg = Com.pg = gtemp+n ;
    Com.lastfvalues = pg+n ; /* size asaParm->m */
    Com.lo = lo ;
    Com.hi = hi ;
    Com.cbbiter = 0 ;
    Com.cbbfunc = 0 ;
    Com.cbbgrad = 0 ;
    Com.cgiter = 0 ;
    Com.cgfunc = 0 ;
    Com.cggrad = 0 ;

    ident = FALSE ;
    xnorm = ZERO ;
    for (j = 0; j < n; j++)
    {
        t = x [j] ;
        if      ( t > hi [j] ) t = hi [j] ;
        else if ( t < lo [j] ) t = lo [j] ;
        x [j] = t ;
        if ( xnorm < fabs (t) ) xnorm = fabs (t) ;
    }

    Com.f = asa_fg (g, x, &Com) ;
    pgnorm = ZERO ;
    gnorm = ZERO ;
    for (j = 0; j < n; j++)
    {
        xj = x [j] ;
        gj = g [j] ;
        xg = xj - gj ;
        if      ( xg > hi [j] ) xp = hi [j] - xj ;
        else if ( xg < lo [j] ) xp = lo [j] - xj ;
        else                    xp = -gj ;
        pg [j] = xp ;
        pgnorm = MAX (pgnorm, fabs (xp)) ;
        gnorm = MAX (gnorm, fabs (gj)) ;
    }
    if ( asaParm->StopRule ) Com.tol = MAX (pgnorm*asaParm->StopFac, grad_tol) ;
    else                     Com.tol = grad_tol ;

    Com.pgnorm = Com.pgnorm_start = pgnorm ;
    if ( asa_tol (pgnorm, &Com) )
    {
        status = 0 ;
        goto Exit ;
    }

    if ( xnorm != ZERO ) Com.alpha = alpha = xnorm/gnorm ;
    else                 Com.alpha = alpha = ONE/gnorm ;

    /* compute gradient norm for inactive variables */
    ginorm = ZERO ;
    nfree = 0 ;
    gp = FALSE ;
    for (j = 0; j < n; j++)
    {
        xj = x [j] ;
        tl = lo [j] ;
        th = hi [j] ;
        gj = g [j] ;
        xg = xj - alpha*gj ;
        if      ( (xg >= th) && (th-xj > pert_hi) ) gp = TRUE ;
        else if ( (xg <= tl) && (xj-tl > pert_lo) ) gp = TRUE ;
        if ( (xj-tl > pert_lo) && (th - xj > pert_hi) )
        {
            ginorm = MAX (ginorm, fabs (gj)) ;
            ifree [nfree] = j ;
            nfree++ ;
        }
    }
    Com.ginorm = ginorm ;
    Com.nfree = nfree ;

    if ( asaParm->PrintLevel >= 1 )
    {
        printf ("\ninitial f = %14.6e pgnorm = %14.6e ginorm = %14.6e\n",
                 Com.f, pgnorm, ginorm) ;
        printf ("            nfree = %i xnorm = %14.6e gp = %i\n",
                 nfree, xnorm, gp) ;
    }

    if ( (ginorm < Com.tau1*pgnorm) || gp || asaParm->GradProjOnly )
    {
        Com.cbbfunc = 1 ;
        Com.cbbgrad = 1 ;
        goto Grad_proj ;
    }
    else
    {
        Com.cgfunc = 1 ;
        Com.cggrad = 1 ;
        goto CG_descent ;
    }

    Grad_proj:
    if ( asaParm->PrintLevel >= 1 ) printf ("\nGradProj:\n") ;
    Com.DimReduce = FALSE ;
    status = asa_grad_proj(&Com) ;
    if ( asaParm->PrintLevel >= 1 )
    {
        printf ("exit Grad_proj\n") ;
    }
    if ( Com.cbbiter >= cbb_totit ) status = 11 ;
    if ( status >= 0 ) goto Exit ;

    /* extract free variable */
    nfree = 0 ;
    for (j = 0; j < n; j++)
    {
        xj = x [j] ;
        if ( (xj-lo [j] > pert_lo) && (hi [j] - xj > pert_hi) )
        {
            ifree [nfree] = j ;
            nfree++ ;
        }
    }
    Com.nfree = nfree ;

    CG_descent:
    if ( nfree != n )
    {
       asa_shrink_all (&Com) ;
       asa_copy (xtemp+nfree, x+nfree, n-nfree) ;
       Com.DimReduce = TRUE ;
    }
    else Com.DimReduce = FALSE ;

    if ( asaParm->PrintLevel >= 1 ) printf ("\nCG:\n") ;
    status = asa_descent (&Com) ;

    if ( asaParm->PrintLevel >= 1 )
    {
        printf ("exit the CG subroutine\n") ;
    }
    if ( Com.DimReduce ) asa_expand_all (&Com) ;
    if ( Com.cgiter >= cg_totit ) status = 2 ;

    if ( status >= -2 ) goto Exit ;

    /* ginorm < tau2* pgnorm without hitting boundary */
    if ( status == -5 )
    {
        Com.alpha = asa_init_bbstep (&Com) ;
        goto Grad_proj ;

    }
    /* ginorm >= tau2* pgnorm and many components of x hit boundary  */
    else if ( status == -4 )
    {
        ginorm = ZERO ;
        nfree = 0 ;
        for (j = 0 ; j < n; j++)
        {
            xj = x [j] ;
            if ( (xj-lo [j] > pert_lo) && (hi [j] - xj > pert_hi) )
            {
                t = fabs (g [j]) ;
                ginorm = MAX (ginorm, t) ;
                ifree [nfree] = j ;
                nfree++ ;
            }
        }
        Com.nfree = nfree ;
        Com.ginorm = ginorm ;

        if ( ginorm >= Com.tau1*Com.pgnorm ) goto CG_descent ;
        else
        {
           if ( asaParm->PrintLevel >= 1 ) printf ("ginorm < tau1* pgnorm\n") ;
           Com.alpha = asa_init_bbstep (&Com) ;
           goto Grad_proj ;
        }
    }
    /* ginorm >= tau2* pgnorm and only one component of x hits boundary */
    else if ( status == -3 )
    {
        if ( pgnorm < asaParm->pgdecay*MAX (Com.pgnorm_start, ONE) )
        {
            ident = asa_identify (x, g, Com.pgnorm, &Com) ;
        }
        if ( ident )
        {
            ident = FALSE ;
            ginorm = ZERO ;
            nfree = 0 ;
            for (j = 0 ; j < n; j++)
            {
                xj = x [j] ;
                if ( (xj-lo [j] > pert_lo) && (hi [j] - xj > pert_hi) )
                {
                    t = fabs (g [j]) ;
                    ginorm = MAX (ginorm, t) ;
                    ifree [nfree] = j ;
                    nfree++ ;
                }
            }
            Com.nfree = nfree ;
            Com.ginorm = ginorm ;
            if ( ginorm >= Com.tau1*Com.pgnorm ) goto CG_descent ;
            else
            {
               if ( asaParm->PrintLevel >= 1 )
                   printf ("ginorm < tau1* pgnorm\n" ) ;
               Com.alpha = asa_init_bbstep (&Com) ;
               goto Grad_proj ;
            }
        }
        else
        {
            Com.alpha = asa_init_bbstep (&Com) ;
            goto Grad_proj ;
        }
    }

    Exit:
    if ( (asaParm->PrintFinal) || (asaParm->PrintLevel >= 1) )
    {
        const char mess1 [] = "Possible causes of this error message:" ;
        const char mess2 [] = "   - your tolerance may be too strict: "
                              "grad_tol = " ;
        const char mess4 [] = "   - your gradient routine has an error" ;
        const char mess5 [] = "   - the parameter epsilon in "
                              "asa_descent_c.parm is too small" ;
        printf ("\nFinal convergence status = %d\n", status);
        if ( status == -2 )
        {
            printf ("Function value became nan at cg iteration %10.0e\n",
                     (double) Com.cgiter) ;
        }
        else if ( status == -1 )
        {
            printf ("Function value of starting point is nan at "
                     "cg iteration %10.0f\n", (double) Com.cgiter) ;
        }
        else if ( status == 0 )
        {
            printf ("Convergence tolerance for gradient satisfied\n") ;
        }
        else if ( status == 1 )
        {
            printf ("Terminating in cg since change in function value "
                    "<= feps*|f|\n") ;
        }
        else if ( status == 2 )
        {
            printf ("Number of iterations exceed specified limits "
                    "for cg routine\n") ;
            printf ("Iterations: %10.0f maxit: %10.0f totit: %10.0f\n",
                    (double) Com.cgiter, (double) Com.cgmaxit,
                    (double) cg_totit) ;
            printf ("%s\n", mess1) ;
            printf ("%s %e\n", mess2, Com.tol) ;
        }
        else if ( status == 3 )
        {
            printf ("Slope always negative in cg line search\n") ;
            printf ("%s\n", mess1) ;
            printf ("   - your cost function has an error\n") ;
            printf ("%s\n", mess4) ;
        }
        else if ( status == 4 )
        {
            printf ("Line search fails in cg, too many secant steps\n") ;
            printf ("%s\n", mess1) ;
            printf ("%s %e\n", mess2, Com.tol) ;
        }
        else if ( status == 5 )
        {
            printf ("Search direction not a descent direction in cg\n") ;
        }
        else if ( status == 6 ) /* line search fails */
        {
            printf ("Line search fails in cg iteration\n") ;
            printf ("%s\n", mess1) ;
            printf ("%s %e\n", mess2, Com.tol) ;
            printf ("%s\n", mess4) ;
            printf ("%s\n", mess5) ;
        }
        else if ( status == 7 ) /* line search fails */
        {
            printf ("Line search fails in cg iteration\n") ;
            printf ("%s\n", mess1) ;
            printf ("%s %e\n", mess2, Com.tol) ;
        }
        else if ( status == 8 ) /* line search fails */
        {
            printf ("Line search fails in cg iteration\n") ;
            printf ("%s\n", mess1) ;
            printf ("%s %e\n", mess2, Com.tol) ;
            printf ("%s\n", mess4) ;
            printf ("%s\n", mess5) ;
        }
        else if ( status == 9 )
        {
            printf ("Debugger is on, function value does not improve in cg\n") ;
            printf ("new value: %25.16e old value: %25.16e\n",
                Com.f_debug, Com.f0) ;
        }
        else if ( status == 10 )
        {
            printf ("Insufficient memory\n") ;
        }
        else if ( status == 11 )
        {
            printf ("Number of iterations or function evaluation exceed\n"
                          "specified limits for cbb routine\n") ;
            printf ("Iterations: %i maxit: %i totit: %i\n",
                     Com.cbbiter, Com.pgmaxit, cbb_totit) ;
            printf ("Total function evaluations: %i maxfunc: %i\n",
                     Com.nf, Com.pgmaxfunc);
        }
        if ( status == 12 ) /* line search fails in cbb iteration */
        {
            printf ("Line search fails in cbb iteration\n") ;
            printf ("%s\n", mess1) ;
            printf ("%s %e\n", mess2, Com.tol) ;
            printf ("%s\n", mess4) ;
        }

        if ( status == 13 )
        {
            printf ("Search direction not descent direction in "
                    "asa_grad_proj\n") ;
            printf ("directional derivative: %e\n", Com.gtd) ;
        }
        if ( status == 14 )
        {
             printf ("At cbb iteration %i function value became nan\n",
                      Com.cbbiter) ;
        }

        printf ("projected gradient max norm: %13.6e\n", Com.pgnorm) ;
        printf ("function value:              %13.6e\n", Com.f) ;
        printf ("\nTotal cg  iterations:           %10.0f\n",
                (double) Com.cgiter) ;
        printf ("Total cg  function evaluations: %10.0f\n",
                (double) Com.cgfunc) ;
        printf ("Total cg  gradient evaluations: %10.0f\n",
                (double) Com.cggrad) ;
        printf ("Total cbb iterations:           %10.0f\n",
                (double) Com.cbbiter) ;
        printf ("Total cbb function evaluations: %10.0f\n",
                (double) Com.cbbfunc) ;
        printf ("Total cbb gradient evaluations: %10.0f\n",
                    (double) Com.cbbgrad) ;
        printf ("------------------------------------------\n") ;
        printf ("Total function evaluations:     %10.0f\n",
                (double) Com.nf) ;
        printf ("Total gradient evaluations:     %10.0f\n",
                (double) Com.ng) ;
        printf ("==========================================\n\n") ;
    }
    free (ifree) ;
    if ( Work == NULL ) free (work) ;
    if ( Stat != NULL )
    {
        Stat->f = Com.f ;
        Stat->pgnorm = Com.pgnorm ;
        Stat->cgiter = Com.cgiter ;
        Stat->cgfunc = Com.cgfunc ;
        Stat->cggrad = Com.cggrad ;
        Stat->cbbiter = Com.cbbiter ;
        Stat->cbbfunc = Com.cbbfunc ;
        Stat->cbbgrad = Com.cbbgrad ;
    }
    return (status) ;
}

/* =========================================================================
   === asa_descent =========================================================
   =========================================================================
   cg_descent conjugate gradient algorithm with modifications to handle the
   bound constraints. Based on cg_descent Version 4.0.
   ========================================================================= */
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
)
{
    int     i, iter, j, maxit, n, n5, nfree, nf, ng, nrestart,
            IterRestart, IterQuad, status ;
    double  delta2, Qk, Ck, pgnorm, ginorm,
            f, ftemp, gnorm, xnorm, gnorm2, dnorm2, denom, bdist,
            t, dphi, dphi0, alpha, talpha,
            xj, gj, xg, xp, sts, sty, sk, xi,
            yk, ykyk, ykgk, dkyk, beta, QuadTrust,
            *x, *d, *g, *xtemp, *gtemp, *lo, *hi, *pg ;

    asacg_parm *Parm ;
    asa_parm *asaParm ;

/* initialization */

    x = Com->x ;
    lo = Com->lo ;
    hi = Com->hi ;
    n = Com->n ;
    d = Com->d ;
    g = Com->g ;
    xtemp = Com->xtemp ;
    gtemp = Com->gtemp ;
    pg = Com->pg ;
    nfree = Com->nfree ;
    nf = Com->nf ;
    ng = Com->ng ;
    pgnorm = Com->pgnorm ;
    ginorm = Com->ginorm ;
    Parm = Com->cgParm ;
    asaParm = Com->asaParm ;

    if ( Parm->PrintLevel >= 1 )
    {
        printf ("Dimension in CG, nfree = %i\n", nfree) ;
    }

    /* the conjugate gradient algorithm is restarted every nrestart iteration */
    nrestart = (INT) (((double) nfree)*Parm->restart_fac) ;

    /* abort when number of iterations reaches maxit in one pass through cg */
    if ( Parm->maxit_fac == INF ) Com->cgmaxit = maxit = INT_INF ;
    else Com->cgmaxit = maxit = (INT) (((double) n)*Parm->maxit_fac) ;

    n5 = nfree % 5 ;
    f = Com->f ;
    Com->f0 = f + f ;
    Ck = ZERO ;
    Qk = ZERO ;

    /* compute inf-norm of x and distance to bounds */
    xnorm = ZERO ;
    bdist = INF ;
    for (i = 0; i < nfree; i++)
    {
        xi = x [i] ;
        if ( xnorm < fabs (xi) ) xnorm = fabs (xi) ;
        t = xi - lo [i] ;
        if ( bdist > t ) bdist = t ;
        t = hi [i] - xi ;
        if ( bdist > t ) bdist = t ;
    }
    Com->bdist = bdist ;

    /* initial function and gradient evaluations, initial direction */
    gnorm = ZERO ;
    gnorm2 = ZERO ;
    for (i = 0; i < n5; i++)
    {
        t = g [i] ;
        d [i] = -t ;
        gnorm2 += t*t ;
        if ( gnorm < fabs (t) ) gnorm = fabs (t) ;
    }
    for (; i < nfree;)
    {
        t = g [i] ;
        d [i] = -t ;
        gnorm2 += t*t ;
        if ( gnorm < fabs (t) ) gnorm = fabs (t) ;
        i++ ;

        t = g [i] ;
        d [i] = -t ;
        gnorm2 += t*t ;
        if ( gnorm < fabs (t) ) gnorm = fabs (t) ;
        i++ ;

        t = g [i] ;
        d [i] = -t ;
        gnorm2 += t*t ;
        if ( gnorm < fabs (t) ) gnorm = fabs (t) ;
        i++ ;

        t = g [i] ;
        d [i] = -t ;
        gnorm2 += t*t ;
        if ( gnorm < fabs (t) ) gnorm = fabs (t) ;
        i++ ;

        t = g [i] ;
        d [i] = -t ;
        gnorm2 += t*t ;
        if ( gnorm < fabs (t) ) gnorm = fabs (t) ;
        i++ ;
    }
    dnorm2 = gnorm2 ;

    /* check whether starting function value is nan */
    if ( f != f )
    {
        status = -1 ;
        goto Exit ;
    }

    if ( Parm->PrintLevel >= 2 )
    {
        printf ("iter: %5i f = %14.6e pgnorm = %14.6e ginorm = %14.6e\n",
          (int) 0, f, pgnorm, ginorm) ;
    }

    dphi0 = -gnorm2 ;
    delta2 = 2*Parm->delta - ONE ;
    alpha = Parm->step ;
    if ( alpha == ZERO )
    {
        alpha = Parm->psi0*xnorm/gnorm ;
        if ( xnorm == ZERO )
        {
            if ( f != ZERO ) alpha = Parm->psi0*fabs (f)/gnorm2 ;
            else             alpha = ONE ;
        }
    }
    IterRestart = 0 ;  /* counts number of iterations since last restart */
    IterQuad = 0 ;     /* counts number of iterations that function change
                          is close to that of a quadratic */

/*  start the conjugate gradient iteration
    alpha starts as old step, ends as final step for current iteration
    f is function value for alpha = 0
    Com->QuadOK = TRUE means that a quadratic step was taken */

    for (iter = 1; iter <= maxit; iter++)
    {
        Com->QuadOK = FALSE ;
        alpha = Parm->psi2*alpha ;
        if ( Com->bdist > 0 )
        {
            Com->minflag = FALSE ;
            Com->minstep = Com->bdist/sqrt(dnorm2) ;
        }
        else asa_maxstep (x, d, Com) ;
        if ( Parm->QuadStep )
        {
            if ( f != ZERO ) t = fabs ((f-Com->f0)/f) ;
            else             t = ONE ;
            if ( t > Parm->QuadCutOff )       /* take provisional step talpha */
            {
                talpha = Parm->psi1*alpha ;
                t = asaParm->parm1*talpha ;
                if ( Com->minstep < t )
                {
                    if ( !Com->minflag ) asa_maxstep (x, d, Com) ;
                }
                if ( Com->minstep >= t )
                {

                    t = MIN (talpha, asaParm->parm2*Com->minstep) ;
                    if ( t < talpha )
                    {
                        if ( !Com->minflag )
                        {
                            asa_maxstep (x, d, Com) ;
                            talpha = MIN (talpha, asaParm->parm2*Com->minstep) ;
                        }
                        else talpha = t ;
                    }
                    asa_step (xtemp, x, d, talpha, nfree) ;
                    /*provisional function value*/
                    ftemp = asa_f (xtemp, Com) ;

                    /* check if function value is nan */
                    if ( ftemp != ftemp ) /* reduce stepsize */
                    {
                        for (i = 0; i < Parm->nexpand; i++)
                        {
                            talpha *= Parm->nan_decay ;
                            asa_step (xtemp, x, d, talpha, nfree) ;
                            ftemp = asa_f (xtemp, Com) ;
                            if ( ftemp == ftemp ) break ;
                        }
                        if ( i == Parm->nexpand )
                        {
                            status = -2 ;
                            goto Exit ;
                        }
                    }

                    if ( ftemp < f )              /* check if QuadStep > 0 */
                    {
                        denom = 2.*(((ftemp-f)/talpha)-dphi0) ;
                        if ( denom > ZERO )    /* try a quadratic fit step */
                        {
                            Com->QuadOK = TRUE ;
                            alpha = -dphi0*talpha/denom ;
                        }
                    }
                }
            }
        }
        Com->f0 = f ;

        if ( Parm->PrintLevel >= 3 )
        {
            if ( Com->minflag )
            {
                printf ("minstep =%14.6e, maxstep =%14.6e\n",
                         Com->minstep, Com->maxstep) ;
            }
            else printf ("bdist  =%14.6e\n", Com->bdist) ;
            printf ("QuadOK: %2i initial a: %14.6e f0: %14.6e dphi0: %14.6e\n",
                    Com->QuadOK, alpha, Com->f0, dphi0) ;
            if ( (alpha > Com->minstep) && Com->QuadOK )
            {
                printf("Quadratic step > minstep to boundary\n") ;
            }
        }

        /* parameters in Wolfe, approximate Wolfe conditions, and in update */
        Qk = Parm->Qdecay*Qk + ONE ;
        Ck = Ck + (fabs (f) - Ck)/Qk ;        /* average cost magnitude */

        if ( Parm->PertRule ) Com->fpert = f + Com->eps*fabs (f) ;
        else                  Com->fpert = f + Com->eps ;

        Com->wolfe_hi = Parm->delta*dphi0 ;
        Com->wolfe_lo = Parm->sigma*dphi0 ;
        Com->awolfe_hi = delta2*dphi0 ;
        Com->alpha = alpha ;/* either double prior step or quadratic fit step */
        Com->f = f ;

        if ( Com->AWolfe )                  /* approximate Wolfe line search*/
        {
            if ( Parm->PrintLevel >= 3 )
            {
                printf ("Perform approximate Wolfe line search\n") ;
            }

            status = asa_line (dphi0, Com) ;
        }
        else                                  /* ordinary Wolfe line search */
        {
            if ( Parm->PrintLevel >= 3 )
            {
                 printf ("Perform ordinary Wolfe line search\n") ;
            }
            status = asa_lineW (dphi0, Com) ;
        }
        /* if ordinary Wolfe line search fails, possibly try approximate
           Wolfe line search*/
        if ( (status > 0) && !Com->AWolfe && (Parm->AWolfeFac > ZERO) )
        {
            Com->AWolfe = TRUE ;
            if ( Parm->PrintLevel >= 3 )
            {
                printf ("Ordinary Wolfe line search fails, "
                        "try approximate Wolfe line search\n") ;
            }

            status = asa_line (dphi0, Com) ;
        }

        alpha = Com->alpha ;
        f = Com->f ;
        dphi = Com->df ;

        if ( (status > 0) || (status == -1) || (status == -2) ) goto Exit ;

        /*Test for convergence to within machine epsilon
          [set feps to zero to remove this test] */

        if ( (-alpha*dphi0 <= Parm->feps*fabs (f)) && (status == 0) )
        {
            status = 1 ;
            goto Exit ;
        }

        /* test how close the cost function changes are to that of a quadratic
           QuadTrust = 0 means the function change matches that of a quadratic*/
        t = alpha*(dphi+dphi0) ;
        if ( fabs (t) <= Parm->qeps*MIN (Ck, ONE) ) QuadTrust = ZERO ;
        else QuadTrust = fabs((2.0*(f-Com->f0)/t)-ONE) ;
        if ( QuadTrust <= Parm->qrule) IterQuad++ ;
        else                           IterQuad = 0 ;

        IterRestart++ ;
        /* test if the CG algorithm should be restarted */
        if ( (IterRestart == nrestart) ||
             ((IterQuad == Parm->qrestart) && (IterQuad != IterRestart) &&
               Parm->AdaptiveBeta) )
        {
            IterRestart = 0 ;
            IterQuad = 0 ;
            /* search direction d = -g */
            if ( Parm->PrintLevel >= 3 ) printf ("RESTART CG\n") ;
            ginorm = ZERO ;
            gnorm2 = ZERO ;
            Com->bdist -= alpha*sqrt(dnorm2) ;
            for (j = 0; j < nfree; j++)
            {
                xj = xtemp [j] ;
                gj = gtemp [j] ;
                d [j] = -gj ;
                gnorm2 += gj*gj ;
                t = fabs (gj) ;
                if ( Com->bdist < t ) break ; /* check for active constraint */
                ginorm = MAX (ginorm, t) ;
            }
            pgnorm = ginorm ;
            if ( j < nfree )
            {
                ginorm = t ;
                xg = xj - gj ;
                if      ( xg > hi [j] ) xp = hi [j] - xj ;
                else if ( xg < lo [j] ) xp = xj - lo [j] ;
                else                    xp = t ;
                pgnorm = MAX (pgnorm, xp) ;
                for (j++; j < nfree; j++)
                {
                    xj = xtemp [j] ;
                    gj = gtemp [j] ;
                    d [j] = -gj ;
                    ginorm = MAX (ginorm, fabs (gj)) ;
                    gnorm2 += gj*gj ;
                    xg = xj - gj ;
                    if      ( xg > hi [j] ) xp = hi [j] - xj ;
                    else if ( xg < lo [j] ) xp = xj - lo [j] ;
                    else                    xp = fabs (gj) ;
                    pgnorm = MAX (pgnorm, xp) ;
                }
            }
            for (; j < n; j++)
            {
                xj = x [j] ;
                gj = gtemp [j] ;
                xg = xj - gj ;
                if      ( xg > hi [j] ) xp = hi [j] - xj ;
                else if ( xg < lo [j] ) xp = xj - lo [j] ;
                else                    xp = fabs (gj) ;
                pgnorm = MAX (pgnorm, xp) ;
            }
            if ( asa_tol (pgnorm, Com) )
            {
                status = 0 ;
                for (j = 0; j < nfree; j++)
                {
                    xj = xtemp [j] ;
                    x [j] = xj ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    g [j] = gj ;
                    if      ( xg > hi [j] ) pg [j] = hi [j] - xj ;
                    else if ( xg < lo [j] ) pg [j] = lo [j] - xj ;
                    else                    pg [j] = -gj ;
                }
                for (; j < n; j++)
                {
                    xj = x [j] ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    g [j] = gj ;
                    if      ( xg > hi [j] ) pg [j] = hi [j] - xj ;
                    else if ( xg < lo [j] ) pg [j] = lo [j] - xj ;
                    else                    pg [j] = -gj ;
                }
                goto Exit1 ;
            }
            if ( ginorm < pgnorm*Com->tau2 ) status = -5 ;
            if ( status < -2 )
            {
                sts = ZERO ;
                sty = ZERO ;
                for (j = 0; j < nfree; j++)
                {
                    t = xtemp[j] ;
                    sk = t - x [j] ;
                    x [j] = t ;
                    sts += sk*sk ;

                    t = gtemp [j] ;
                    sty += sk*(t-g [j]) ;
                    g [j] = t ;
                }
                Com->sts = sts ;
                Com->sty = sty ;
                goto Exit ;
            }

            dphi0 = -gnorm2 ;
            asa_copy (x, xtemp, nfree) ;
            asa_copy (g, gtemp, nfree) ;
            dnorm2 = gnorm2 ;
        }
        else /* compute beta, yk2, gnorm, gnorm2, dnorm2, update x and g */
        {
            ginorm = ZERO ;
            Com->bdist -= alpha*sqrt(dnorm2) ;
            for (j = 0; j < nfree; j++)
            {
                t = fabs (gtemp [j]) ;
                if ( Com->bdist < t ) break ; /* check for active constraint */
                ginorm = MAX (ginorm, t) ;
            }
            pgnorm = ginorm ;
            if ( j < nfree )
            {
                ginorm = t ;
                xj = xtemp [j] ;
                xg = xj - gtemp [j] ;
                if      ( xg > hi [j] ) xp = hi [j] - xj ;
                else if ( xg < lo [j] ) xp = xj - lo [j] ;
                else                    xp = t ;
                pgnorm = MAX (pgnorm, xp) ;
                for (j++; j < nfree; j++)
                {
                    xj = xtemp [j] ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    if      ( xg > hi [j] ) xp = hi [j] - xj ;
                    else if ( xg < lo [j] ) xp = xj - lo [j] ;
                    else                    xp = fabs (gj) ;
                    pgnorm = MAX (pgnorm, xp) ;
                    ginorm = MAX (ginorm, fabs (gj)) ;
                }
            }
            for (; j < n; j++)
            {
                xj = x [j] ;
                gj = gtemp [j] ;
                xg = xj - gj ;
                if      ( xg > hi [j] ) xp = hi [j] - xj ;
                else if ( xg < lo [j] ) xp = xj - lo [j] ;
                else                    xp = fabs (gj) ;
                pgnorm = MAX (pgnorm, xp) ;
            }
            if ( asa_tol (pgnorm, Com) )
            {
                status = 0 ;
                for (j = 0; j < nfree; j++)
                {
                    xj = xtemp [j] ;
                    x [j] = xj ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    g [j] = gj ;
                    if      ( xg > hi [j] ) pg [j] = hi [j] - xj ;
                    else if ( xg < lo [j] ) pg [j] = lo [j] - xj ;
                    else                    pg [j] = -gj ;
                }
                for (; j < n; j++)
                {
                    xj = x [j] ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    g [j] = gj ;
                    if      ( xg > hi [j] ) pg [j] = hi [j] - xj ;
                    else if ( xg < lo [j] ) pg [j] = lo [j] - xj ;
                    else                    pg [j] = -gj ;
                }
                goto Exit1 ;
            }
            if ( ginorm < pgnorm*Com->tau2 ) status = -5 ;
            if ( status < -2 )
            {
                sts = ZERO ;
                sty = ZERO ;
                for (j = 0; j < nfree; j++)
                {
                    t = xtemp[j] ;
                    sk = t - x [j] ;
                    x [j] = t ;
                    sts += sk*sk ;

                    t = gtemp [j] ;
                    sty += sk*(t-g [j]) ;
                    g [j] = t ;
                }
                Com->sts = sts ;
                Com->sty = sty ;
                goto Exit ;
            }

            asa_copy (x, xtemp, nfree) ;
            ykyk = ZERO ;
            ykgk = ZERO ;
            for (j = 0; j < n5; j++)
            {
                t = gtemp [j] ;
                yk = t - g [j] ;
                g [j] = t ;
                ykyk += yk*yk ;
                ykgk += yk*t ;
            }
            for (j = n5; j < nfree; )
            {
                t = gtemp [j] ;
                yk = t - g [j] ;
                g [j] = t ;
                ykyk += yk*yk ;
                ykgk += yk*t ;
                j++ ;

                t = gtemp [j] ;
                yk = t - g [j] ;
                g [j] = t ;
                ykyk += yk*yk ;
                ykgk += yk*t ;
                j++ ;

                t = gtemp [j] ;
                yk = t - g [j] ;
                g [j] = t ;
                ykyk += yk*yk ;
                ykgk += yk*t ;
                j++ ;

                t = gtemp [j] ;
                yk = t - g [j] ;
                g [j] = t ;
                ykyk += yk*yk ;
                ykgk += yk*t ;
                j++ ;

                t = gtemp [j] ;
                yk = t - g [j] ;
                g [j] = t ;
                ykyk += yk*yk ;
                ykgk += yk*t ;
                j++ ;
            }

            dkyk = dphi - dphi0 ;
            if ( Parm->AdaptiveBeta ) t = 2. - ONE/(0.1*QuadTrust + ONE) ;
            else                      t = Parm->theta ;
            beta = (ykgk - t*dphi*ykyk/dkyk)/dkyk ;
            /* faster: initialize dnorm2 = gnorm2 at start, then
                       dnorm2 = gnorm2 + beta**2*dnorm2 - 2.*beta*dphi
                       gnorm2 = ||g_{k+1}||^2
                       dnorm2 = ||d_{k+1}||^2
                       dpi = g_{k+1}' d_k */

            /* lower bound for beta is BetaLower*d_k'g_k/ ||d_k||^2 */
            beta = MAX (beta, Parm->BetaLower*dphi0/dnorm2) ;

            /* update search direction d = -g + beta*dold */
            gnorm2 = ZERO ;
            dnorm2 = ZERO ;
            for (i = 0; i < n5; i++)
            {
                t = g [i] ;
                gnorm2 += t*t ;
                t = -t + beta*d [i] ;
                d [i] = t ;
                dnorm2 += t*t ;
            }
            for (; i < nfree; )
            {
                t = g [i] ;
                gnorm2 += t*t ;
                t = -t + beta*d [i] ;
                d [i] = t ;
                dnorm2 += t*t ;
                i++ ;

                t = g [i] ;
                gnorm2 += t*t ;
                t = -t + beta*d [i] ;
                d [i] = t ;
                dnorm2 += t*t ;
                i++ ;

                t = g [i] ;
                gnorm2 += t*t ;
                t = -t + beta*d [i] ;
                d [i] = t ;
                dnorm2 += t*t ;
                i++ ;

                t = g [i] ;
                gnorm2 += t*t ;
                t = -t + beta*d [i] ;
                d [i] = t ;
                dnorm2 += t*t ;
                i++ ;

                t = g [i] ;
                gnorm2 += t*t ;
                t = -t + beta*d [i] ;
                d [i] = t ;
                dnorm2 += t*t ;
                i++ ;
            }
            dphi0 = -gnorm2 + beta*dphi ;
            if ( Parm->debug ) /* Check the dphi0 = d'g */
            {
                t = ZERO ;
                for (j=0; j<nfree; j++)  t = t + d[j]*g[j] ;
                if ( fabs(t-dphi0) > Parm->debugtol*fabs(dphi0) )
                {
                    printf("Warning, dphi0 != d'g!\n");
                    printf("dphi0:%14.6e, d'g:%14.6e\n",dphi0, t) ;
                }
            }
        }

        if ( !Com->AWolfe )
        {
            if ( fabs (f-Com->f0) <= Parm->AWolfeFac*Ck ) Com->AWolfe = TRUE ;
        }

        if ( Parm->PrintLevel >= 2 )
        {
            printf ("\niter: %5i f = %14.6e pgnorm = %14.6e ginorm = %14.6e\n",
              (int) iter, f, pgnorm, ginorm) ;
        }

        if ( Parm->debug )
        {
            if ( f > Com->f0 + Ck*Parm->debugtol )
            {
                status = 9 ;
                goto Exit ;
            }
        }

        if ( dphi0 > ZERO )
        {
           status = 5 ;
           goto Exit ;
        }
    }
    status = 2 ;

Exit:
    if ( status < -2 )
    {
        for (j = nfree; j < n; j++) g [j] = gtemp [j] ;
    }
    else
    {
        pgnorm = ZERO ;
        for (j = 0; j < n; j++)
        {
            xj = xtemp [j] ;
            x [j] = xj ;
            gj = gtemp [j] ;
            g [j] = gj ;
            xg = xj - gj ;
            if      ( xg > hi [j] ) xp = hi [j] - xj ;
            else if ( xg < lo [j] ) xp = lo [j] - xj ;
            else                    xp = -gj ;
            pgnorm = MAX (pgnorm, fabs (xp)) ;
            pg [j] = xp ;
        }
    }

Exit1:
    Com->pgnorm = pgnorm ;
    Com->ginorm = ginorm ;
    Com->f = f ;
    Com->f_debug = f ;
    Com->cgfunc += Com->nf - nf ;
    Com->cggrad += Com->ng - ng ;
    Com->cgiter += iter ;
    if ( Parm->PrintLevel >= 2 )
    {
        printf ("\niter: %5i f = %14.6e pgnorm = %14.6e ginorm = %14.6e\n",
                (int) iter, f, pgnorm, ginorm) ;
    }
    if ( Parm->PrintLevel >= 1 )
    {
        printf ("\nCG Termination status: %i\n", status) ;
        if ( status == -5 )
        {
            printf ("ginorm < tau2*pgnorm without hitting boundary\n") ;
        }
        if ( status == -4 )
        {
            printf ("ginorm >= tau2*pgnorm, many x components hit boundary\n") ;
        }
        else if ( status == -3 )
        {
            printf ("ginorm >= tau2*pgnorm, one x component hits boundary\n") ;
        }
        printf ("proj gradient max norm: %13.6e\n", pgnorm) ;
        printf ("function value:         %13.6e\n", f) ;
        printf ("cg iterations:          %13.6e\n", (double) iter) ;
        printf ("function evaluations:   %13.6e\n", (double) Com->nf - nf) ;
        printf ("gradient evaluations:   %13.6e\n", (double) Com->ng - ng) ;
    }
    return (status) ;
}

/* =========================================================================
   === asa_Wolfe ===========================================================
   =========================================================================
   Check whether the Wolfe or the approximate Wolfe conditions are satisfied
   ========================================================================= */
int asa_Wolfe
(
    double       alpha , /* stepsize */
    double           f , /* function value associated with stepsize alpha */
    double        dphi , /* derivative value associated with stepsize alpha */
    asa_com        *Com  /* cg com */
)
{
    if ( dphi >= Com->wolfe_lo )
    {

        /* test original Wolfe conditions */
        if ( f - Com->f0 <= alpha*Com->wolfe_hi )
        {
            if ( Com->cgParm->PrintLevel >= 4 )
            {
                printf ("wolfe f: %14.6e f0: %14.6e dphi: %14.6e\n",
                         f, Com->f0, dphi) ;
            }
            return (1) ;
        }
        /* test approximate Wolfe conditions */
        else if ( Com->AWolfe )
        {
            if ( (f <= Com->fpert) && (dphi <= Com->awolfe_hi) )
            {
                if ( Com->cgParm->PrintLevel >= 4 )
                {
                    printf ("f: %14.6e fpert: %14.6e dphi: %14.6e awolf_hi: "
                            "%14.6e\n", f, Com->fpert, dphi, Com->awolfe_hi) ;
                }
                return (1) ;
            }
        }
    }
    return (0) ;
}

/* =========================================================================
   === asa_tol =============================================================
   =========================================================================
   Check for convergence
   ========================================================================= */
int asa_tol
(
    double      pgnorm, /* projected gradient sup-norm */
    asa_com       *Com
)
{
    /*StopRule = T => |grad|_infty <=max (tol, |grad|_infty*StopFac)
                 F => |grad|_infty <= tol*(1+|f|)) */
    if ( Com->asaParm->StopRule )
    {
        if ( pgnorm <= Com->tol ) return (1) ;
    }
    else if ( pgnorm <= Com->tol*(ONE + fabs (Com->f)) ) return (1) ;
    return (0) ;
}

/* =========================================================================
   === asa_step ============================================================
   =========================================================================
   Compute xtemp = x + alpha d
   ========================================================================= */
void asa_step
(
    double *xtemp , /*output vector */
    double     *x , /* initial vector */
    double     *d , /* search direction */
    double  alpha , /* stepsize */
    INT         n   /* length of the vectors */
)
{
    INT n5, i ;
    n5 = n % 5 ;
    for (i = 0; i < n5; i++) xtemp [i] = x[i] + alpha*d[i] ;
    for (; i < n; i += 5)
    {
        xtemp [i]   = x [i]   + alpha*d [i] ;
        xtemp [i+1] = x [i+1] + alpha*d [i+1] ;
        xtemp [i+2] = x [i+2] + alpha*d [i+2] ;
        xtemp [i+3] = x [i+3] + alpha*d [i+3] ;
        xtemp [i+4] = x [i+4] + alpha*d [i+4] ;
    }
}

/* =========================================================================
   === asa_line ============================================================
   =========================================================================
   Approximate Wolfe line search routine
   ========================================================================= */
int asa_line
(
    double       dphi0, /* function derivative at starting point (alpha = 0) */
    asa_com       *Com  /* cg com structure */
)
{
    int i, iter, nfree, nsecant, nshrink, ngrow, status ;
    double a, dphia, b, dphib, c, alpha, alpha1, phi, phi1, dphi, dphi1,
           alphaold, phiold, a0, da0, b0, db0, width, fquad, rho, minstep, t,
           *x, *xtemp, *d, *gtemp ;
    asacg_parm *Parm ;

    nfree = Com->nfree ;
    x = Com->x ;         /* current iterate */
    d = Com->d ;         /* current search direction */
    xtemp = Com->xtemp ; /* x + alpha*d */
    gtemp = Com->gtemp ; /* gradient at x + alpha*d */
    minstep = Com->minstep ;
    alpha = Com->alpha ;
    if ( alpha > minstep )
    {
        if ( !Com->minflag )
        {
            asa_maxstep (x, d, Com) ;
            minstep = Com->minstep ;
            if ( alpha > minstep )
            {
               alpha = minstep ;
               Com->QuadOK = FALSE ;
            }
        }
        else
        {
            alpha = minstep ;
            Com->QuadOK = FALSE ;
        }
    }
    phi = Com->f ;
    Parm = Com->cgParm ;
    rho = Parm->rho ;
    asa_step (xtemp, x, d, alpha, nfree) ;
    asa_g (gtemp, xtemp, Com) ;
    dphi = asa_dot (gtemp, d, nfree) ;

    /* check if gradient is nan; if so, reduce stepsize */
    if ( dphi != dphi )
    {
        for (i = 0; i < Parm->nexpand; i++)
        {
            alpha *= Parm->nan_decay ;
            asa_step (xtemp, x, d, alpha, nfree) ;
            asa_g (gtemp, xtemp, Com) ;
            dphi = asa_dot (gtemp, d, nfree) ;
            if ( dphi == dphi ) break ;
        }
        if ( i == Parm->nexpand )
        {
            status = -2 ;
            goto Exit ;
        }
        Com->QuadOK = FALSE ;
        rho = Parm->nan_rho ;
    }

/*Find initial interval [a,b] such that dphia < 0, dphib >= 0,
         and phia <= phi0 + feps*Ck */

    a = ZERO ;
    dphia = dphi0  ;
    ngrow = 0 ;
    nshrink = 0 ;
    while ( dphi < ZERO )
    {
        phi = asa_f (xtemp, Com) ;

/* if QuadStep in effect and quadratic conditions hold, check wolfe condition*/

        if ( Com->QuadOK )
        {
            if ( ngrow == 0 ) fquad = MIN (phi, Com->f0) ;
            if ( phi <= fquad )
            {
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("alpha: %14.6e phi: %14.6e fquad: %14.6e\n",
                            alpha, phi, fquad) ;
                }
                if ( asa_Wolfe (alpha, phi, dphi, Com) )
                {
                    status = 0 ;
                    goto Exit ;
                }
            }
        }
        if ( phi > Com->fpert )
        {
            /* contraction phase, only break when more than Parm->nshrink
               iterations or Secant step */
            b = alpha ;
            alpha1 = alpha ;
            phi1 = phi ;
            dphi1 = dphi ;
            while ( TRUE )
            {
                alpha = .5*(a+b) ;
                nshrink++ ;
                if ( nshrink > Parm->nexpand )
                {
                    status = 6 ;
                    goto Exit ;
                }

                /* if slope remains negative after Parm->nshrink tries,
                   then expand the search interval starting from alpha1 */
                if ( nshrink > Parm->nshrink )
                {
                    t = Com->f0 ;
                    if ( Parm->PertRule )
                    {
                        if ( t != ZERO )
                        {
                            Com->eps = Parm->egrow*(phi1-t)/fabs (t) ;
                            Com->fpert = t + fabs (t)*Com->eps ;
                        }
                        else Com->fpert = 2.*phi1 ;
                    }
                    else
                    {
                        Com->eps = Parm->egrow*(phi1-t) ;
                        Com->fpert = t + Com->eps ;
                    }
                    if ( Parm->PrintLevel >= 1 )
                    {
                        printf ("increase eps to: %e fpert: %e\n",
                                 Com->eps, Com->fpert) ;
                    }
                    alpha = alpha1 ;
                    dphi = dphi1 ;
                    break ;
                }

                asa_step (xtemp, x, d, alpha, nfree) ;
                asa_g (gtemp, xtemp, Com) ;
                dphi = asa_dot (gtemp, d, nfree) ;
                if ( dphi >= ZERO ) goto Secant ;
                phi = asa_f (xtemp, Com) ;
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("contract, a: %14.6e b: %14.6e alpha: %14.6e phi: "
                            "%14.6e dphi: %14.6e\n", a, b, alpha, phi, dphi) ;
                }
                if ( Com->QuadOK && (phi <= fquad) )
                {
                    if ( asa_Wolfe (alpha, phi, dphi, Com) )
                    {
                        status = 0 ;
                        goto Exit ;
                    }
                }
                if ( phi <= Com->fpert )
                {
                    a = alpha ;
                    dphia = dphi ;
                }
                else
                {
                    b = alpha ;
                }
            }
        }

/* expansion phase */
        a = alpha ;
        dphia = dphi ;
        ngrow++ ;
        if ( ngrow > Parm->nexpand )
        {
            status = 3 ;
            goto Exit ;
        }
        alphaold = alpha ;
        alpha = rho*alpha ;
        if ( alpha > minstep )
        {
            if ( !Com->minflag )
            {
                asa_maxstep (x, d, Com) ;
                minstep = Com->minstep ;
                if ( alpha > minstep ) alpha = minstep ;
            }
            else  alpha = minstep ;
        }
        if ( alpha != alphaold )
        {
            asa_step (xtemp, x, d, alpha, nfree) ;
            asa_g (gtemp, xtemp, Com) ;
            dphi = asa_dot (gtemp, d, nfree) ;
            if ( Parm->PrintLevel >= 4 )
            {
                printf ("expand,   a: %14.6e alpha: %14.6e phi: "
                         "%14.6e dphi: %14.6e\n", a, alpha, phi, dphi) ;
            }
        }
        else /* a new constraint is active */
        {
            do /* while statement */
            {
                alphaold = alpha ;
                phiold = phi ;
                if ( alpha < Com->maxstep )
                {
                    alpha = rho*alphaold ;
                    asa_project (xtemp, x, d, alpha, Com) ;
                    phi = asa_f (xtemp, Com) ;
                }
            } while ( phi < phiold ) ;
            if ( alphaold == minstep )
            {
                asa_step (xtemp, x, d, minstep, nfree) ;
                status = -3 ;
            }
            else
            {
                asa_project (xtemp, x, d, alphaold, Com) ;
                asa_g (gtemp, xtemp, Com) ;
                status = -4 ;
            }
            phi = phiold ;
            goto Exit ;
        }
    }

Secant:
    b = alpha ;
    dphib = dphi ;
    if ( Com->QuadOK )
    {
        phi = asa_f (xtemp, Com) ;
        if ( ngrow + nshrink == 0 ) fquad = MIN (phi, Com->f0) ;
        if ( phi <= fquad )
        {
            if ( asa_Wolfe (alpha, phi, dphi, Com) )
            {
                status = 0 ;
                goto Exit ;
            }
        }
    }
    nsecant = Parm->nsecant ;
    for (iter = 1; iter <= nsecant; iter++)
    {
        if ( Parm->PrintLevel >= 4 )
        {
            printf ("secant, a: %14.6e b: %14.6e da: %14.6e db: %14.6e\n",
                     a, b, dphia, dphib) ;
        }
        width = Parm->gamma*(b - a) ;
        if ( -dphia <= dphib ) alpha = a - (a-b)*(dphia/(dphia-dphib)) ;
        else                   alpha = b - (a-b)*(dphib/(dphia-dphib)) ;
        a0 = a ;
        b0 = b ;
        da0 = dphia ;
        db0 = dphib ;
        status = asa_update (&a, &dphia, &b, &dphib, &alpha, &phi,
                    &dphi, Com) ;
        if ( status >= 0 ) goto Exit ;
        c = alpha ;
        if ( status == -2 )
        {
            if ( c == a )
            {
                if ( dphi > da0 ) alpha = c - (c-a0)*(dphi/(dphi-da0)) ;
                else              alpha = a ;
            }
            else
            {
                if ( dphi < db0 ) alpha = c - (c-b0)*(dphi/(dphi-db0)) ;
                else              alpha = b ;
            }
            if ( (alpha > a) && (alpha < b) )
            {
                if ( Parm->PrintLevel >= 4 ) printf ("2nd secant\n") ;
                status = asa_update (&a, &dphia, &b, &dphib, &alpha, &phi,
                          &dphi, Com) ;
                if ( status >= 0 ) goto Exit ;
            }
        }

/* bisection iteration */

        if ( b-a >= width )
        {
            alpha = .5*(b+a) ;
            if ( Parm->PrintLevel >= 4 ) printf ("bisection\n") ;
            status = asa_update (&a, &dphia, &b, &dphib, &alpha, &phi,
                        &dphi, Com) ;
            if ( status >= 0 ) goto Exit ;
        }
        else if ( b <= a )
        {
            status = 7 ;
            goto Exit ;
        }
    }
    status = 4 ;

Exit:
    Com->alpha = alpha ;
    Com->f = phi ;
    Com->df = dphi ;
    return (status) ;
}

/* =========================================================================
   === asa_update ==========================================================
   =========================================================================
   update returns: 8 if too many iterations
                   0 if Wolfe condition is satisfied
                  -1 if interval is updated and a search is done
                  -2 if the interval updated successfully
   ========================================================================= */
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
)
{
    int nfree, nshrink, status ;
    double A, B, dphiA, dphiB, phiA, t, *x, *d, *xtemp, *gtemp ;
    asacg_parm *Parm ;

    nfree = Com->nfree ;
    x = Com->x ;         /* current iterate */
    d = Com->d ;         /* current search direction */
    xtemp = Com->xtemp ; /* x + alpha*d */
    gtemp = Com->gtemp ; /* gradient at x + alpha*d */
    Parm = Com->cgParm ;
    asa_step (xtemp, x, d, *alpha, nfree) ;
    *phi = asa_fg (gtemp, xtemp, Com) ;
    *dphi = asa_dot (gtemp, d, nfree) ;
    if ( Parm->PrintLevel >= 4 )
    {
        printf ("update alpha: %14.6e phi: %14.6e dphi: %14.6e\n",
                 *alpha, *phi, *dphi) ;
    }
    if ( asa_Wolfe (*alpha, *phi, *dphi, Com) ) status = 0 ;
    else
    {
        status = -2 ;
        if ( *dphi >= ZERO )
        {
            *b = *alpha ;
            *dphib = *dphi ;
        }
        else if ( *phi <= Com->fpert )
        {
            *a = *alpha ;
            *dphia = *dphi ;
        }
        else
        {
           /* store right side of current interval in case of later failure */
            B = *b ;
            dphiB = *dphib ;
            A = *alpha ;
            phiA = *phi ;
            dphiA = *dphi ;

            /* apply bisection to interval [a, alpha] */
            nshrink = 0 ;
            *b = *alpha ;
            while ( TRUE )
            {
                *alpha = .5*(*a + *b) ;
                nshrink++ ;

                /* if slope remains negative after Parm->nshrink tries,
                   then increase eps and accept right side of initial interval*/
                if ( nshrink > Parm->nshrink )
                {
                    t = Com->f0 ;
                    if ( Parm->PertRule )
                    {
                         if ( t != ZERO )
                         {
                             Com->eps = Parm->egrow*(phiA-t)/fabs (t) ;
                             Com->fpert = t + fabs (t)*Com->eps ;
                         }
                         else Com->fpert = 2.*phiA ;
                    }
                    else
                    {
                        Com->eps = Parm->egrow*(phiA-t) ;
                        Com->fpert = t + Com->eps ;
                    }
                    if ( Parm->PrintLevel >= 1 )
                    {
                        printf ("increase eps to: %e fpert: %e\n",
                                 Com->eps, Com->fpert) ;
                    }
                    *a = A ;
                    *alpha = A ;
                    *dphia = dphiA ;
                    *b = B ;
                    *dphib = dphiB ;
                    break ;
                }

                if ( nshrink > Parm->nexpand )
                {
                    status = 8 ;
                    break ;
                }
                asa_step (xtemp, x, d, *alpha, nfree) ;
                *phi = asa_fg (gtemp, xtemp, Com) ;
                *dphi = asa_dot (gtemp, d, nfree) ;
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("contract, a: %14.6e alpha: %14.6e phi: %14.6e "
                            "dphi: %14.6e\n", *a, *alpha, *phi, *dphi) ;
                }
                if ( asa_Wolfe (*alpha, *phi, *dphi, Com) )
                {
                    status = 0 ;
                    break ;
                }
                if ( *dphi >= ZERO )
                {
                    *b = *alpha ;
                    *dphib = *dphi ;
                    status = -1 ;
                    break ;
                }
                if ( *phi <= Com->fpert )
                {
                    if ( Parm->PrintLevel >= 4 )
                    {
                        printf ("update a: %14.6e dphia: %14.6e\n",
                                 *alpha, *dphi) ;
                    }
                    *a = *alpha ;
                    *dphia = *dphi ;
                }
                else *b = *alpha ;
            }
        }
    }
    if ( Parm->PrintLevel >= 3 )
    {
        printf ("UP a: %14.6e b: %14.6e da: %14.6e db: %14.6e status: %i\n",
                 *a, *b, *dphia, *dphib, status) ;
    }
    return (status) ;
}

/* =========================================================================
   === asa_lineW ===========================================================
   =========================================================================
   Ordinary Wolfe line search routine.
   This routine is identical to asa_line except that the function
   psi [a] = phi [a] - phi [0] - a*delta*dphi [0] is minimized instead of
   the function phi
   ========================================================================= */
int asa_lineW
(
    double       dphi0 , /* function derivative at starting point (alpha = 0) */
    asa_com       *Com   /* cg com structure */
)
{
    int i, iter, nfree, nsecant, nshrink, ngrow, status ;
    double a, dpsia, b, dpsib, c, alpha, alpha1, phi, dphi, alphaold, phiold,
           a0, da0, b0, db0, width, fquad, rho, psi, psi1, dpsi, dpsi1,
           minstep, t, *x, *d, *xtemp, *gtemp ;
    asacg_parm *Parm ;

    nfree = Com->nfree ;
    x = Com->x ;         /* current iterate */
    d = Com->d ;         /* current search direction */
    xtemp = Com->xtemp ; /* x + alpha*d */
    gtemp = Com->gtemp ; /* gradient at x + alpha*d */
    minstep = Com->minstep ;
    alpha = Com->alpha ;
    if ( alpha > minstep )
    {
        if ( !Com->minflag )
        {
            asa_maxstep (x, d, Com) ;
            minstep = Com->minstep ;
            if ( alpha > minstep )
            {
               alpha = minstep ;
               Com->QuadOK = FALSE ;
            }
        }
        else
        {
            alpha = minstep ;
            Com->QuadOK = FALSE ;
        }
    }
    phi = Com->f ;
    Parm = Com->cgParm ;
    rho = Parm->rho ;
    asa_step (xtemp, x, d, alpha, nfree) ;
    asa_g (gtemp, xtemp, Com) ;
    dphi = asa_dot (gtemp, d, nfree) ;

    /* check if gradient is nan; if so, reduce stepsize */
    if ( dphi != dphi )
    {
        for (i = 0; i < Parm->nexpand; i++)
        {
            alpha *= Parm->nan_decay ;
            asa_step (xtemp, x, d, alpha, nfree) ;
            asa_g (gtemp, xtemp, Com) ;
            dphi = asa_dot (gtemp, d, nfree) ;
            if ( dphi == dphi ) break ;
        }
        if ( i == Parm->nexpand )
        {
            status = -2 ;
            goto Exit ;
        }
        Com->QuadOK = FALSE ;
        rho = Parm->nan_rho ;
    }
    dpsi = dphi - Com->wolfe_hi ;

    /*Find initial interval [a,b] such that dphia < 0, dphib >= 0,
         and phia <= phi0 + feps*Ck */

    a = ZERO ;
    dpsia = dphi0 - Com->wolfe_hi ;
    ngrow = 0 ;
    nshrink = 0 ;
    while ( dpsi < ZERO )
    {
        phi = asa_f (xtemp, Com) ;
        psi = phi - alpha*Com->wolfe_hi ;

        /* if QuadStep in effect and quadratic conditions hold,
           check Wolfe condition*/

        if ( Com->QuadOK )
        {
            if ( ngrow == 0 ) fquad = MIN (phi, Com->f0) ;
            if ( phi <= fquad )
            {
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("alpha: %14.6e phi: %14.6e fquad: %14.6e\n",
                            alpha, phi, fquad) ;
                }
                if ( asa_Wolfe (alpha, phi, dphi, Com) )
                {
                    status = 0 ;
                    goto Exit ;
                }
            }
        }
        if ( psi > Com->fpert )
        {
            /* contraction phase, only break when more than Parm->nshrink
               iterations or Secant step */
            b = alpha ;
            alpha1 = alpha ;
            psi1 = psi ;
            dpsi1 = dpsi ;
            while ( TRUE )
            {
                alpha = .5*(a+b) ;
                nshrink++ ;
                if ( nshrink > Parm->nexpand )
                {
                    status = 6 ;
                    goto Exit ;
                }

               /* if slope remains negative after Parm->nshrink tries,
                   then expand the search interval starting from alpha1 */
                if ( nshrink > Parm->nshrink )
                {
                    t = Com->f0 ;
                    if ( Parm->PertRule )
                    {
                        if ( t != ZERO )
                        {
                            Com->eps = Parm->egrow*(psi1-t)/fabs (t) ;
                            Com->fpert = t + fabs (t)*Com->eps ;
                        }
                        else Com->fpert = 2.*psi1 ;
                    }
                    else
                    {
                        Com->eps = Parm->egrow*(psi1-t) ;
                        Com->fpert = t + Com->eps ;
                    }
                    alpha = alpha1 ;
                    psi = psi1 ;
                    dpsi = dpsi1 ;
                    if ( Parm->PrintLevel >= 1 )
                    {
                        printf ("increase eps to: %e fpert: %e\n",
                                 Com->eps, Com->fpert) ;
                    }
                    break ;
                }

                asa_step (xtemp, x, d, alpha, nfree) ;
                asa_g (gtemp, xtemp, Com) ;
                dphi = asa_dot (gtemp, d, nfree) ;
                dpsi = dphi - Com->wolfe_hi ;
                if ( dpsi >= ZERO ) goto Secant ;
                phi = asa_f (xtemp, Com) ;
                psi = phi - alpha*Com->wolfe_hi ;
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("contract, a: %14.6e b: %14.6e alpha: %14.6e phi: "
                            "%14.6e dphi: %14.6e\n", a, b, alpha, phi, dphi) ;
                }
                if ( Com->QuadOK && (phi <= fquad) )
                {
                    if ( asa_Wolfe (alpha, phi, dphi, Com) )
                    {
                        status = 0 ;
                        goto Exit ;
                    }
                }
                if ( psi <= Com->fpert )
                {
                    a = alpha ;
                    dpsia = dpsi ;
                }
                else
                {
                    b = alpha ;
                }
            }
        }

/* expansion phase */
        a = alpha ;
        dpsia = dpsi ;
        ngrow++ ;
        if ( ngrow > Parm->nexpand )
        {
            status = 3 ;
            goto Exit ;
        }
        alphaold = alpha ;
        alpha = rho*alpha ;
        if ( alpha > minstep )
        {
            if ( !Com->minflag )
            {
                asa_maxstep (x, d, Com) ;
                minstep = Com->minstep ;
                if ( alpha > minstep ) alpha = minstep ;
            }
            else  alpha = minstep ;
        }
        if ( alpha != alphaold )
        {
            asa_step (xtemp, x, d, alpha, nfree) ;
            asa_g (gtemp, xtemp, Com) ;
            dphi = asa_dot (gtemp, d, nfree) ;
            dpsi = dphi - Com->wolfe_hi ;
            if ( Parm->PrintLevel >= 4 )
            {
                printf ("expand,   a: %14.6e alpha: %14.6e phi: "
                         "%14.6e dphi: %14.6e\n", a, alpha, phi, dphi) ;
            }
        }
        else /* a new constraint is active */
        {
            do /* while statement */
            {
                alphaold = alpha ;
                phiold = phi ;
                if ( alpha < Com->maxstep )
                {
                    alpha = rho*alphaold ;
                    asa_project (xtemp, x, d, alpha, Com) ;
                    phi = asa_f (xtemp, Com) ;
                }
            } while ( phi < phiold ) ;
            if ( alphaold == minstep )
            {
                asa_step (xtemp, x, d, minstep, nfree) ;
                status = -3 ;
            }
            else
            {
                asa_project (xtemp, x, d, alphaold, Com) ;
                asa_g (gtemp, xtemp, Com) ;
                status = -4 ;
            }
            phi = phiold ;
            goto Exit ;
        }
    }

Secant:
    b = alpha ;
    dpsib = dpsi ;
    if ( Com->QuadOK )
    {
        phi = asa_f (xtemp, Com) ;
        if ( ngrow + nshrink == 0 ) fquad = MIN (phi, Com->f0) ;
        if ( phi <= fquad )
        {
            if ( asa_Wolfe (alpha, phi, dphi, Com) )
            {
                status = 0 ;
                goto Exit ;
            }
        }
    }
    nsecant = Parm->nsecant ;
    for (iter = 1; iter <= nsecant; iter++)
    {
        if ( Parm->PrintLevel >= 4 )
        {
            printf ("secant, a: %14.6e b: %14.6e da: %14.6e db: %14.6e\n",
                     a, b, dpsia, dpsib) ;
        }
        width = Parm->gamma*(b - a) ;
        if ( -dpsia <= dpsib ) alpha = a - (a-b)*(dpsia/(dpsia-dpsib)) ;
        else                   alpha = b - (a-b)*(dpsib/(dpsia-dpsib)) ;
        a0 = a ;
        b0 = b ;
        da0 = dpsia ;
        db0 = dpsib ;
        status = asa_updateW (&a, &dpsia, &b, &dpsib, &alpha, &phi, &dphi,
                   &dpsi, Com) ;
        if ( status >= 0 ) goto Exit ;
        c = alpha ;
        if ( status == -2 )
        {
            if ( c == a )
            {
                if ( dpsi > da0 ) alpha = c - (c-a0)*(dpsi/(dpsi-da0)) ;
                else              alpha = a ;
            }
            else
            {
                if ( dpsi < db0 ) alpha = c - (c-b0)*(dpsi/(dpsi-db0)) ;
                else              alpha = b ;
            }
            if ( (alpha > a) && (alpha < b) )
            {
                if ( Parm->PrintLevel >= 4 ) printf ("2nd secant\n") ;
                status = asa_updateW (&a, &dpsia, &b, &dpsib, &alpha, &phi,
                   &dphi, &dpsi, Com) ;
                if ( status >= 0 ) goto Exit ;
            }
        }

/* bisection iteration */

        if ( b-a >= width )
        {
            alpha = .5*(b+a) ;
            if ( Parm->PrintLevel >= 4 ) printf ("bisection\n") ;
            status = asa_updateW (&a, &dpsia, &b, &dpsib, &alpha, &phi, &dphi,
                       &dpsi, Com) ;
            if ( status >= 0 ) goto Exit ;
        }
        else if ( b <= a )
        {
            status = 7 ;
            goto Exit ;
        }
    }
    status = 4 ;

Exit:
    Com->alpha = alpha ;
    Com->f = phi ;
    Com->df = dphi ;
    return (status) ;
}

/* =========================================================================
   === asa_updateW =========================================================
   =========================================================================
   This routine is identical to asa_update except that the function
   psi [a] = phi [a] - phi [0] - a*delta*dphi [0] is minimized instead of
   the function phi. The return int has the following meaning:
                   8 if too many iterations
                   0 if Wolfe condition is satisfied
                  -1 if interval is updated and a search is done
                  -2 if the interval updated successfully
   ========================================================================= */
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
)
{
    int nfree, nshrink, status ;
    double A, B, dpsiA, dpsiB, psi, psiA, t ;
    double *x, *d, *xtemp, *gtemp ;
    asacg_parm *Parm ;

    nfree = Com->nfree ;
    x = Com->x ;         /* current iterate */
    d = Com->d ;         /* current search direction */
    xtemp = Com->xtemp ; /* x + alpha*d */
    gtemp = Com->gtemp ; /* gradient at x + alpha*d */
    Parm = Com->cgParm ;
    asa_step (xtemp, x, d, *alpha, nfree) ;
    *phi = asa_fg (gtemp, xtemp, Com) ;
    psi = *phi - *alpha*Com->wolfe_hi ;
    *dphi = asa_dot (gtemp, d, nfree) ;
    *dpsi = *dphi - Com->wolfe_hi ;
    if ( Parm->PrintLevel >= 4 )
    {
        printf ("update alpha: %14.6e psi: %14.6e dpsi: %14.6e\n",
                 *alpha, psi, *dpsi) ;
    }
    if ( asa_Wolfe (*alpha, *phi, *dphi, Com) ) status = 0 ;
    else
    {
        status = -2 ;
        if ( *dpsi >= ZERO )
        {
            *b = *alpha ;
            *dpsib = *dpsi ;
        }
        else if ( psi <= Com->fpert )
        {
            *a = *alpha ;
            *dpsia = *dpsi ;
        }
        else
        { 
            /* store right side of current interval in case of later failure */
            B = *b ;
            dpsiB = *dpsib ;
            A = *alpha ;
            psiA = psi ;
            dpsiA = *dpsi ;

            /* apply bisection to interval [a, alpha] */
            nshrink = 0 ;
            *b = *alpha ;

            while ( TRUE )
            {
                *alpha = .5*(*a + *b) ;
                nshrink++ ;

                /* if slope remains negative after Parm->nshrink tries,
                   then increase eps and accept right side of initial interval*/
                if ( nshrink > Parm->nshrink )
                {
                    t = Com->f0 ;
                    if ( Parm->PertRule )
                    {
                         if ( t != ZERO )
                         {
                             Com->eps = Parm->egrow*(psiA-t)/fabs (t) ;
                             Com->fpert = t + fabs (t)*Com->eps ;
                         }
                         else Com->fpert = 2.*psiA ;
                    }
                    else
                    {
                        Com->eps = Parm->egrow*(psiA-t) ;
                        Com->fpert = t + Com->eps ;
                    }
                    if ( Parm->PrintLevel >= 1 )
                    {
                        printf ("increase eps to: %e fpert: %e\n",
                                 Com->eps, Com->fpert) ;
                    }
                    *a = A ;
                    *alpha = A ;
                    *dpsia = dpsiA ;
                    *dpsi = dpsiA ;
                    *b = B ;
                    *dpsib = dpsiB ;
                    break ;
                }

                if ( nshrink > Parm->nexpand )
                {
                    status = 8 ;
                    break ;
                }
                asa_step (xtemp, x, d, *alpha, nfree) ;
                *phi = asa_fg (gtemp, xtemp, Com) ;
                *dphi = asa_dot (gtemp, d, nfree) ;
                *dpsi = *dphi - Com->wolfe_hi ;
                psi = *phi - *alpha*Com->wolfe_hi ;
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("contract, a: %14.6e alpha: %14.6e phi: %14.6e "
                            "dphi: %14.6e\n", *a, *alpha, *phi, *dphi) ;
                }
                if ( asa_Wolfe (*alpha, *phi, *dphi, Com) )
                {
                    status = 0 ;
                    break ;
                }
                if ( *dpsi >= ZERO )
                {
                    *b = *alpha ;
                    *dpsib = *dpsi ;
                    status = -1 ;
                    break ;
                }
                if ( psi <= Com->fpert )
                {
                    if ( Parm->PrintLevel >= 4 )
                    {
                        printf ("update a: %14.6e dpsia: %14.6e\n",
                                *alpha, *dpsi) ;
                    }
                    *a = *alpha ;
                    *dpsia = *dpsi ;
                }
                else *b = *alpha ;
            }
        }
    }
    if ( Parm->PrintLevel >= 3 )
    {
        printf ("UP a: %14.6e b: %14.6e da: %14.6e db: %14.6e status: %i\n",
                 *a, *b, *dpsia, *dpsib, status) ;
    }
    return (status) ;
}

/* =========================================================================
   === asa_project =========================================================
   =========================================================================
   Project a vector into the feasible set
   ========================================================================= */
void asa_project
(
    double  *xnew,
    double     *x,
    double     *d,
    double  alpha,
    asa_com  *Com   /* cg com structure */
)
{
    int j, n ;
    double t, *lo, *hi ;
    lo = Com->lo ;
    hi = Com->hi ;
    n = Com->nfree ;
    for (j = 0; j < n; j++)
    {
        t = x [j] + alpha*d [j] ;
        if      ( t > hi [j] ) t = hi [j] ;
        else if ( t < lo [j] ) t = lo [j] ;
        xnew [j] = t ;
    }
}

/* =========================================================================
   === asa_maxstep =========================================================
   =========================================================================
   Compute maximum step in the search direction until hitting boundary
   ========================================================================= */
void asa_maxstep
(
    double       *x, /* current iterate */
    double       *d, /* direction */
    asa_com    *Com
)
{
    double bdist, step, minstep, maxstep, xj, t, *lo, *hi ;
    int j, n ;

    Com->minflag = TRUE ;
    n = Com->nfree ;
    minstep = INF ;
    maxstep = ZERO ;
    bdist = INF ;
    lo = Com->lo ;
    hi = Com->hi ;

    for (j = 0;  j < n; j++)
    {
        xj = x [j] ;
        if ( d [j] > ZERO )
        {
            if ( hi [j] < INF )
            {
                t = hi [j] - xj ;
                if ( bdist > t ) bdist = t ;
                step = t/d [j] ;
                minstep = MIN (minstep, step) ;
                maxstep = MAX (maxstep, step) ;
            }
            t = xj - lo [j] ;
            if ( bdist > t ) bdist = t ;
        }
        else if ( d [j] < ZERO )
        {
            if ( lo [j] >-INF )
            {
                t = xj - lo [j] ;
                if ( bdist > t ) bdist = t ;
                step = -t/d [j] ;
                minstep = MIN (minstep, step) ;
                maxstep = MAX (maxstep, step) ;
            }
            t = hi [j] - xj ;
            if ( bdist > t ) bdist = t ;
        }
        else
        {
            t = xj - lo [j] ;
            if ( bdist > t ) bdist = t ;
            t = hi [j] - xj ;
            if ( bdist > t ) bdist = t ;
        }
    }
    Com->bdist = bdist ;
    Com->minstep = minstep ;
    Com->maxstep = maxstep ;
    return ;
}

/* =========================================================================
   === asa_grad_proj =======================================================
   =========================================================================
   Nonmonotone gradient projection algorithm modified to combine with cg_descent
   for bound constraints problem.

   Notation:
   fmin  = min {f(x_i),  0 <= i <= k}, k is current iteration (best value)
   fmax  = max {f_{k-i}, i = 0, 1, ... , min(k,m-1)}, m = memory
   fr    = reference function value
   f     = f(x_k), current function value
   fc    = maximum objective function value since the last minimum
          (best) function value was found. In other words, if
          k is the current iteration f(k_1) = fmin, then
          fc = max {f(x_i), k_1 <= i <= k}
   fcomp = min {fr, fmax}
   fr_pert = fr + pert, pert = Parm->eps*|fcomp| or Parm->eps
                        depending on PertRule
   fcomp_pert = fcomp + pert
   ftemp = f(xtemp), temporary (or a trial) function value at xtemp
   ========================================================================= */
int asa_grad_proj /*return:
                      -1 (give active constraints to cg routine)
                       0 (convergence tolerance satisfied)
                      11 (number of iterations or function evaluations
                          exceed limit)
                      12 (line search fails)
                      13 (search direction in linesearch is not descent)
                      14 (function value became nan) */
(
    asa_com *Com
)
{
    int count, i, ident, index, iter, j, ll, ll0, mcount, mm, n, nf, nf_line,
        ng, nl, np, status, hitbound, getbound, freebound ;
    double alpha, armijo_decay, armijo0, armijo1, f, fmin, fc, fr, sts, gtd,
           fmax, lambda, pgnorm, ginorm, gnorm, xnorm,
           xj, gj, xp, xg, t, th, tl,
           pert_lo, pert_hi, atemp, ftemp, fcomp, sty, yty, s, y, cosine,
           *lo, *hi, *x, *d, *g, *xtemp, *gtemp, *pg, *lastfvalues ;
    double fr_pert, fcomp_pert, dphia, Armijo_hi, AArmijo_hi ;
    asa_parm *Parm ;

    n = Com->n ;
    x = Com->x ;
    lo = Com->lo ;
    hi = Com->hi ;
    d = Com->d ;
    g = Com->g ;
    xtemp = Com->xtemp ;
    gtemp = Com->gtemp ;
    pg = Com->pg ;
    lastfvalues = Com->lastfvalues ;
    pgnorm = Com->pgnorm ;
    ginorm = Com->ginorm ;
    nf = Com->nf ;
    ng = Com->ng ;
    Parm = Com->asaParm ;
    pert_lo = Parm->pert_lo ;
    pert_hi = Parm->pert_hi ;
    armijo_decay = Parm->armijo_decay ;
    armijo0 = Parm->armijo0 ;
    armijo1 = Parm->armijo1 ;
    f = Com->f ;

    iter = 0 ;
    status = 0 ;
    count = 0 ;
    mcount = 2 ;
    ident = FALSE ;
    lambda = Com->alpha ;
    lastfvalues [0] = f ;
    for (i = 1; i < Parm->m; i++) lastfvalues [i] = -INF ;
    mm = 0 ; /* number of iterations in the current CBB cycle */
    ll = 0 ; /* zero as long as unit steps in line search */
    nl = 0 ; /* number of iterations since fmin decreased in value */
    np = 0 ; /* number of times initial stepsize was accepted in line search */
    fmin = f ;
    fr = f ;
    fc = f ;

    if ( Parm->PrintLevel >= 3 )
    {
        printf ("Initial stepsize in cbb: %14.6e\n", lambda) ;
    }

    while ( TRUE )
    {
        if ( Parm->PrintLevel >= 2 )
        {
            printf ("cbb iter: %5i f: %14.6e pgnorm: %14.6e\n\n",
                     iter, f, pgnorm) ;
        }

        if ( !Parm->GradProjOnly )
        {
            if ( ginorm >= Com->tau1*pgnorm )
            {
               if ( ident || (count >= mcount) )
               {
                   status = -1 ;
                   goto Exit ;
               }
            }
            else
            {
                if ( ident )
                {
                    ident = FALSE ;
                    Com->tau1 *= Parm->tau1_decay ;
                    Com->tau2 *= Parm->tau2_decay ;
                }
            }
        }
        iter++ ;
        hitbound = FALSE ;
        getbound = FALSE ;
        freebound = FALSE ;
        sts = ZERO ;
        gtd = ZERO ;
        for (j = 0; j < n; j++)
        {
            xj = x [j] ;
            gj = g [j] ;
            xp = -lambda*gj ;
            xg = xj + xp ;
            th = hi [j] ;
            tl = lo [j] ;
            if ( xg >= th )
            {
                xp = th - xj ;
                xtemp [j] = th ;
                if ( xp > pert_hi ) getbound = TRUE ;
            }
            else if ( xg <= tl )
            {
                xp = tl - xj ;
                xtemp [j] = tl ;
                if ( -xp > pert_lo ) getbound = TRUE ;
            }
            else
            {
                xtemp [j] = xg ;
                if ( (xj == th) || (xj == tl) ) freebound = TRUE ;
            }
            d [j] = xp ;
            gtd += gj*xp ; /* g'd (derivative in search direction) */
            sts += xp*xp ;
        }
        if ( getbound ) ll++ ;
        nf_line = Com->nf ;

        if (gtd >= ZERO)
        {
            status = 13 ;
            Com->gtd = gtd ;
            goto Exit ;
        }

       /* start of cbb line search */
        if ( Parm->PrintLevel >= 4 )
        {
            printf ("Linesearch in cbb, f: %14.6e gtd: %14.6e\n", f, gtd) ;
        }
        fmax = lastfvalues [0] ;
        for (i = 1; i < Parm->m; i++) fmax = MAX (fmax, lastfvalues [i]) ;
        alpha = ONE ;
        ftemp = asa_f (xtemp, Com) ;

        if ( nl == Parm->L )
        {
            fr = fmax ;
            t = (fr-fmin)/(fc-fmin) ;
            if ( t > Parm->gamma1 ) fr = fc ;
            nl = 0 ;
        }

        if ( (np > Parm->P) && (fmax > f) )
        {
           t = (fr-f)/(fmax-f) ;
           if ( t > Parm->gamma2 ) fr = fmax ;
        }

        fcomp = MIN (fmax, fr) ;

        if ( Parm->PrintLevel >= 4 )
        {
            printf ("fr: %14.6e fcomp: %14.6e\n", fr, fcomp) ;
        }

        /* Approximate nonmonotone Armijo line search, decrease alpha until:
           phi'(alpha) <= [2(phi_r - phi(0))/alpha] + (2 delta - 1) phi'(0) and
           phi(alpha) <= phi_r, where phi_r = fr_pert or fcomp_pert. */
        if ( Com->AArmijo)
        {
            if ( Parm->PertRule ) t = Parm->eps*fabs(fcomp) ;
            else                  t = Parm->eps ;
            fr_pert = fr + t ;
            fr = fr_pert ;
            fcomp_pert = fcomp + t ;
            if ( Parm->PrintLevel >= 3 )
            {
                printf ("Perform approximate Armijo line search\n") ;
                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("fr_pert: %14.6e fcomp_pert: %14.6e\n",
                             fr_pert, fcomp_pert) ;
                }
            }

            AArmijo_hi = (TWO*Parm->delta - ONE)*gtd ;
            if ( ftemp != ftemp ) /* function value is nan, reduce stepsize */
            {
                for (i = 0; i < Parm->nshrink; i++)
                {
                    ll++ ;
                    alpha *= Parm->nan_fac ;
                    asa_step (xtemp, x, d, alpha, n) ;
                    ftemp = asa_f (xtemp, Com) ;
                    if ( ftemp == ftemp ) break ;
                }
                if ( (i == Parm->nshrink) || (alpha == ZERO) )
                {
                    status = 14 ;
                    goto exit_with_error ;
                }

                if ( ftemp <= fcomp_pert)
                {
                    asa_g (gtemp, xtemp, Com) ;
                    dphia = asa_dot (gtemp, d, n) ;
                    if (dphia <= TWO*(fcomp_pert - f)/alpha + AArmijo_hi )
                        goto exit_cbbls ; /* unit step is valid */
                }
            }
            else
            {
                if ( mm == 0 )
                {
                    if ( ftemp <= fr_pert )
                    {
                        asa_g (gtemp, xtemp, Com) ;
                        dphia = asa_dot (gtemp, d, n) ;
                        if (dphia <= TWO*(fr_pert - f) + AArmijo_hi )
                        {
                            mm++ ;
                            goto exit_cbbls ;
                        }
                    }
                }
                else
                {
                    if ( ftemp <= fcomp_pert )
                    {
                        asa_g (gtemp, xtemp, Com) ;
                        dphia = asa_dot (gtemp, d, n) ;
                        if (dphia <= TWO*(fcomp_pert - f) + AArmijo_hi )
                        {
                            mm++ ;
                            goto exit_cbbls ;
                        }
                    }
                }
            }

            /* backtracking approximate nonmonotone line search */
            ll0 = ll ;
            while ( TRUE )
            {
                /* Modified Raydan's quadratic interpolation line search */
                t = TWO*(ftemp-f-alpha*gtd) ;
                if ( t != ZERO )
                {
                    atemp = (-gtd*alpha*alpha)/t ;
                    if ( (atemp < armijo0*alpha) || (atemp > armijo1*alpha ) )
                    {
                        atemp = armijo_decay*alpha ;
                    }
                    alpha = atemp ;
                }
                else alpha *= armijo_decay ;

                asa_saxpy (xtemp, x, d, alpha, n) ; /* xtemp = x + alpha*d */
                ftemp = asa_f (xtemp, Com) ;
                ll++ ;

                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("alpha: %14.6e ftemp: %14.6e\n", alpha, ftemp) ;
                }

                if ( ftemp <= fcomp_pert )
                {
                    asa_g (gtemp, xtemp, Com) ;
                    dphia = asa_dot (gtemp, d, n) ;
                    if (dphia <= TWO*(fcomp_pert - f)/alpha + AArmijo_hi )
                        goto exit_cbbls ;
                }

                if ( (alpha <= ZERO) || (ll-ll0 >= Parm->max_backsteps) )
                {
                   status = 12 ;
                   goto exit_with_error ;
                }
            }
            /* End of approximate Armijo line search */
        }

        /* Ordinary nonmonotone Armijo line search, decrease alpha until
           phi(alpha) <= phi_r + alpha * delta * phi'(0)
           where phi_r = fr or fcomp. */
        else
        {
            if ( Parm->PrintLevel >= 3 )
            {
                printf ("Perform ordinary Armijo line search\n") ;
            }

            Armijo_hi = Parm->delta*gtd ;
            if ( ftemp != ftemp ) /* function value is nan, reduce stepsize */
            {
                for (i = 0; i < Parm->nshrink; i++)
                {
                    ll++ ;
                    alpha *= Parm->nan_fac ;
                    asa_step (xtemp, x, d, alpha, n) ;
                    ftemp = asa_f (xtemp, Com) ;
                    if ( ftemp == ftemp ) break ;
                }
                if ( (i == Parm->nshrink) || (alpha == ZERO) )
                {
                    status = 14 ;
                    goto exit_with_error ;
                }
                if ( ftemp <= fcomp+alpha*Armijo_hi ) goto exit_cbbls ;
            }
            else
            {
                if ( mm == 0 ) t = fr ;
                else           t = fcomp ;
                if ( ftemp <= t+Armijo_hi )
                {
                    mm++ ;
                    goto exit_cbbls ;
                }
            }

            ll0 = ll ;
            while ( TRUE )
            {
                /* Modified Raydan's quadratic interpolation line search */
                t = TWO*(ftemp-f-alpha*gtd) ;
                if ( t != ZERO )
                {
                    atemp = (-gtd*alpha*alpha)/t ;
                    if ( (atemp < armijo0*alpha) || (atemp > armijo1*alpha ) )
                    {
                        atemp = armijo_decay*alpha ;
                    }
                    alpha = atemp ;
                }
                else alpha *= armijo_decay ;

                asa_saxpy (xtemp, x, d, alpha, n) ; /* xtemp = x + alpha*d */
                ftemp = asa_f (xtemp, Com) ;
                ll++ ;

                if ( Parm->PrintLevel >= 4 )
                {
                    printf ("alpha: %14.6e ftemp: %14.6e\n", alpha, ftemp) ;
                }

                if ( ftemp <= fcomp+alpha*Armijo_hi ) break ;

                if ( (alpha <= ZERO) || (ll-ll0 >= Parm->max_backsteps) )
                {
                    /* try approximate Armijo line search  */
                    if ( Parm->AArmijoFac > ZERO ) fr = fcomp ;
                    else                                 /* line search fails */
                    {
                        status = 12 ;
                        goto exit_with_error ;
                    }
                }
            }
            /* End of ordinary Armijo line search */
        }

        exit_cbbls:

        if ( ftemp <= fmin )
        {
             fmin = ftemp ;
             fc = ftemp ;
             nl = 0 ;
        }
        else nl++ ;
        if ( ftemp > fc ) fc = ftemp ;

        exit_with_error:
        /* end of cbbls */

        if ( getbound && (alpha == ONE) ) hitbound = TRUE ;
        if ( Parm->PrintLevel >= 3 )
        {
            printf ("hitbound = %i freebound = %i alpha = %14.6e\n",
                     hitbound, freebound, alpha) ;
        }

        if ( hitbound || freebound ) count = 0 ;
        else                         count++ ;

        sts *= alpha*alpha ;
        if ( Com->nf == nf_line + 1 ) np++ ;
        else                          np = 0 ;

        if ( !Com->AArmijo)  asa_g (gtemp, xtemp, Com) ;

        /* linesearch fails */
        if ( status > 0 )
        {
            if ( ftemp < f )
            {
                f = ftemp ;
                asa_copy(x, xtemp, n) ;
                asa_copy(g, gtemp, n) ;
            }
            pgnorm = ZERO ;
            for (j = 0; j < n; j++)
            {
                xj = x [j] ;
                gj = g [j] ;
                xg = xj - gj ;
                if      ( xg >= hi [j] ) xp = hi [j] - xj ;
                else if ( xg <= lo [j] ) xp = lo [j] - xj ;
                else                     xp = -gj ;
                pgnorm = MAX (pgnorm, fabs (xp)) ;
                pg [j] = xp ;
            }
            goto Exit ;
        }

        index = 0 ;

        if ( (ll >= 1) || (mm >= Parm->nm) || (iter <= 1) )
        {
            index = 1 ;
            sty = ZERO ;
            pgnorm = ZERO ;
            ginorm = ZERO ;
            for (j = 0; j < n; j++)
            {
                xj = xtemp [j] ;
                gj = gtemp [j] ;
                xg = xj - gj ;
                if ( (xj - lo [j] > pert_lo) && (hi [j] - xj > pert_hi) )
                    ginorm = MAX (ginorm, fabs (gj));
                if      ( xg >= hi [j] ) xp = hi [j] - xj ;
                else if ( xg <= lo [j] ) xp = xj - lo [j] ;
                else                     xp = fabs (gj) ;
                pgnorm = MAX (pgnorm, xp) ;
                sty += (xj - x [j])*(gj - g [j]) ;
                x [j] = xj ;
                g [j] = gj ;
            }

            if ( asa_tol (pgnorm, Com) )
            {
                f = ftemp ;
                for (j  = 0; j < n; j++)
                {
                    xj = xtemp [j] ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    if      ( xg >= hi [j] ) xp = hi [j] - xj ;
                    else if ( xg <= lo [j] ) xp = lo [j] - xj ;
                    else                     xp = -gj ;
                    pg [j] = xp ;
                }
                status = 0 ;

                goto Exit ;
            }
        }

        else
        {
            pgnorm = ZERO ;
            ginorm = ZERO ;
            gnorm = ZERO ;
            sty = ZERO ;
            yty = ZERO ;
            for (j = 0; j < n; j++)
            {
                xj = xtemp [j] ;
                gj = gtemp [j] ;
                xg = xj - gj ;
                t = fabs (gj) ;
                gnorm = MAX (gnorm, t) ;
                if      ( xg >= hi [j] ) xp = hi [j] - xj ;
                else if ( xg <= lo [j] ) xp = xj - lo [j] ;
                else                     xp = t ;
                pgnorm = MAX (pgnorm, xp) ;
                if ( (xj - lo [j] > pert_lo) && (hi [j] - xj > pert_hi) )
                {
                    ginorm = MAX (ginorm, t) ;
                }
                s = xj - x [j] ;
                y = gj - g [j] ;
                sty += s*y ;
                yty += y*y ;
                x [j] = xj ;
                g [j] = gj ;
            }
            if ( asa_tol (pgnorm, Com) )
            {
                f = ftemp ;
                for (j  = 0; j < n; j++)
                {
                    xj = xtemp [j] ;
                    gj = gtemp [j] ;
                    xg = xj - gj ;
                    if      ( xg >= hi [j] ) xp = hi [j] - xj ;
                    else if ( xg <= lo [j] ) xp = lo [j] - xj ;
                    else                     xp = -gj ;
                    pg [j] = xp ;
                }
                status = 0 ;
                goto Exit ;
            }
            s = Parm->parm3*fabs (ftemp)/gnorm ;
            t = MAX (s, ONE) ;
            if ( sts > t*t ) index = 1 ;
            else
            {
                t = MIN (s, ONE) ;
                if ( sts <= t*t )
                {
                    cosine = fabs (sty)/sqrt (sts*yty) ;
                    if ( cosine >= Parm->gamma ) index = 1 ;
                }
            }
        }

        if ( index == 1 )
        {
            ll = 0 ;
            if ( sty <= ZERO)
            {
                if ( mm >= Parm->parm4 )
                {
                    xnorm = asa_max (x, n) ;
                    t = MIN (ONE/pgnorm, xnorm/pgnorm) ;
                    lambda = MAX (t, lambda) ;
                    mm = 0 ;
                }
            }
            else
            {
                t = MAX (Parm->lmin, sts/sty) ;
                lambda = MIN (Parm->lmax, t) ;
                mm = 0 ;
            }
        }

        /* If not GradProjOnly, check if the active constraints are identified*/
        if ( !Parm->GradProjOnly &&
              pgnorm < Parm->pgdecay*MAX (ONE, Com->pgnorm_start) )
        {
            ident = asa_identify(x, g, pgnorm, Com) ;
        }

        f = ftemp ;
        lastfvalues [iter % Parm->m] = f ;

        /* check for excessive iterations/function evaluations */
        if ( (iter >= Com->pgmaxit) || (Com->nf - nf  >= Com->pgmaxfunc) )
        {
            for (j = 0; j < n; j++)
            {
                xj = x [j] ;
                gj = g [j] ;
                xg = xj - gj ;
                if      ( xg >= hi [j] ) xp = hi [j] - xj ;
                else if ( xg <= lo [j] ) xp = lo [j] - xj ;
                else                     xp = -gj ;
                pg [j] = xp ;
            }
            status = 11 ;
            goto Exit ;
        }

        if ( !Com->AArmijo )
        {
            if ( fabs(fr - f) <= Parm->AArmijoFac*fabs(fcomp) )
                Com->AArmijo = TRUE ;
        }
    }
    Exit:
    Com->f = f ;
    Com->ginorm = ginorm ;
    Com->pgnorm = pgnorm ;
    Com->cbbiter += iter ;
    Com->cbbfunc += Com->nf - nf ;
    Com->cbbgrad += Com->ng - ng ;
    if ( Parm->PrintLevel >= 2 )
    {
        if(status != -1) printf ("cbb iter: %5i f: %14.6e pgnorm: %14.6e\n\n",
                                  iter, f, pgnorm) ;
    }
    if ( Parm->PrintLevel >= 1 )
    {
        printf ("\nCBB Termination status: %i\n", status) ;
        if ( status == -1 )
            printf ("terminate cbb iteration, branch to cg iteration\n") ;

        printf ("proj gradient max norm: %13.6e\n", pgnorm) ;
        printf ("function value:         %13.6e\n", f) ;
        printf ("cbb iterations:         %13.6e\n", (double) iter) ;
        printf ("function evaluations:   %13.6e\n", (double) Com->nf - nf) ;
        printf ("gradient evaluations:   %13.6e\n", (double) Com->ng - ng) ;
    }
    return (status) ;
}

/* =========================================================================
   === asa_init_bbstep =====================================================
   =========================================================================
   Calculate initial BB stepsize
   ========================================================================= */
double asa_init_bbstep
(
    asa_com *Com
)
{
    int n ;
    double alpha, lmax, lmin, pgnorm, xnorm, sts, sty, t, *x ;
    x = Com->x ;
    sts = Com->sts ;
    sty = Com->sty ;
    pgnorm = Com->pgnorm ;
    n = Com->n ;
    lmin = Com->asaParm->lmin ;
    lmax = Com->asaParm->lmax ;

    if ( sty > ZERO )
    {
        t = MIN (sts/sty, lmax) ;
        alpha = MAX (lmin, t) ;
    }
    else
    {
        xnorm = asa_max (x, n) ;
        if ( xnorm > ZERO ) alpha = MIN (ONE, xnorm)/pgnorm ;
        else                alpha = ONE/pgnorm ;
    }
    return (alpha) ;
}

/* =========================================================================
   ==== asa_f ==============================================================
   Evaluate the function
   =========================================================================*/
double asa_f
(
    double    *x,
    asa_com *Com
)
{
    double f ;
    asa_objective *user ;
    user = Com->user ;
    user->x = x ;
    Com->nf++ ;
    if ( Com->DimReduce )
    {
        /* Expand x to the full space*/
        asa_expandx (x, Com) ;

        /* Evaluate function */
        user->ifree = Com->ifree ;
        user->nfree = Com->nfree ;
        f = Com->value (user) ;

        /* Shrink x to the reduced space */
        asa_shrinkx (x, Com) ;
    }
    else
    {
        /* Evaluate function */
        user->ifree = NULL ;
        user->nfree = Com->n ;
        f = Com->value (user) ;
    }
    return (f) ;

}

/* =========================================================================
   ==== asa_g ==============================================================
   Evaluate the gradient
   =========================================================================*/
void asa_g
(
    double    *g,
    double    *x,
    asa_com *Com
)
{
    asa_objective *user ;
    user = Com->user ;
    user->x = x ;
    user->g = g ;
    Com->ng++ ;
    if ( Com->DimReduce )
    {
        /* Expand x to the full space*/
        asa_expandx (x, Com) ;

        /* Evaluate gradient */
        user->ifree = Com->ifree ;
        user->nfree = Com->nfree ;
        Com->grad (user) ;

        /* Shrink x and g to the reduced space */
        asa_shrinkxg (x, g, Com) ;
    }
    else
    {
        /* Evaluate gradient */
        user->ifree = NULL ;
        user->nfree = Com->n ;
        Com->grad (user) ;
    }
}


/* =========================================================================
   ==== asa_fg =============================================================
   Evaluate the function and gradient
   =========================================================================*/
double asa_fg
(
    double    *g,
    double    *x,
    asa_com *Com
)
{
    asa_objective *user ;
    double f ;
    Com->nf++ ;
    Com->ng++ ;
    user = Com->user ;
    user->x = x ;
    user->g = g ;
    if ( Com->DimReduce )
    {
        /* Expand x to the full space*/
        asa_expandx (x, Com) ;

        /* Evaluate function and gradient */
        user->ifree = Com->ifree ;
        user->nfree = Com->nfree ;
        if ( Com->valgrad != NULL )
        {
            f = Com->valgrad (user) ;
        }
        else
        {
            Com->grad (user) ;
            f = Com->value (user) ;
        }

        /* Shrink x and g to the reduced space */
        asa_shrinkxg (x, g, Com) ;
    }
    else
    {
        /* Evaluate function and gradient */
        user->ifree = NULL ;
        user->nfree = Com->n ;
        if ( Com->valgrad != NULL )
        {
            f = Com->valgrad (user) ;
        }
        else
        {
            Com->grad (user) ;
            f = Com->value (user) ;
        }
    }
    return (f) ;
}

/* =========================================================================
   ==== asa_identify =======================================================
   Check whether the bounds with strict complementarity
   are approximately identified
   =========================================================================*/
int asa_identify
(
   double     *x,
   double     *g,
   double pgnorm,
   asa_com  *Com
)
{
    int ident, j, n ;
    double t, t1, xj, *lo, *hi ;
    n = Com->n ;
    lo = Com->lo ;
    hi = Com->hi ;
    ident = TRUE ;
    t = sqrt (pgnorm) ;
    t1 = t*t*t ;
    for (j = 0; j < n; j++)
    {
        xj = x [j] ;
        if ( ((xj - lo [j] >= t1) && (g [j] > t)) ||
             ((hi [j] - xj >= t1) && (g [j] <-t)) ) ident = FALSE ;
    }
    return (ident) ;
}

/* =========================================================================
   === asa_expandx =========================================================
   =========================================================================
   Expand x array from size nfree to full size of dimension n based on
   indices of free variables
   ========================================================================= */
void asa_expandx
(
    double    *x,
    asa_com *Com
)
{
    int i, j, nfree, *ifree ;
    double t ;
    ifree = Com->ifree ;
    nfree = Com->nfree ;
    for (j = nfree-1; j >= 0; j--)
    {
        i = ifree [j] ;
        if ( j != i )
        {
            t = x [i] ;
            x [i] = x [j] ;
            x [j] = t ;
        }
    }
}

/* =========================================================================
   === asa_shrinkx =========================================================
   =========================================================================
   Compress x array to dimension nfree based on indices of free variables
   ========================================================================= */
void asa_shrinkx
(
    double    *x,
    asa_com *Com
)
{
    int i, j, nfree, *ifree ;
    double t ;
    ifree = Com->ifree ;
    nfree = Com->nfree ;
    for (j = 0; j < nfree; j++)
    {
        i = ifree [j] ;
        if ( j != i )
        {
            t = x [i] ;
            x [i] = x [j] ;
            x [j] = t ;
        }
    }
}

/* =========================================================================
   === asa_shrinkxg ========================================================
   =========================================================================
   Compress x and g arrays based on indices of free variables
   ========================================================================= */
void asa_shrinkxg
(
    double    *x,
    double    *g,
    asa_com *Com
)
{
    int i, j, nfree, *ifree ;
    double t ;
    ifree = Com->ifree ;
    nfree = Com->nfree ;
    for (j = 0; j < nfree; j++)
    {
        i = ifree [j] ;
        if ( j != i )
        {
            t = x [i] ;
            x [i] = x [j] ;
            x [j] = t ;

            t = g [i] ;
            g [i] = g [j] ;
            g [j] = t ;
        }
    }
}

/* =========================================================================
   === asa_expand_all ======================================================
   =========================================================================
   Expand vectors x, g, pg, lo and hi from the reduced space (dimension nfree)
   to the full space (dimension n).
   ========================================================================= */
void asa_expand_all
(
    asa_com *Com
)
{
    int i, j, nfree, *ifree ;
    double t, *x, *g, *pg, *lo, *hi ;
    x = Com->x ;
    g = Com->g ;
    pg = Com->pg ;
    lo = Com->lo ;
    hi = Com->hi ;
    ifree = Com->ifree ;
    nfree = Com->nfree ;
    for (j = nfree-1; j >= 0; j--)
    {
        i = ifree [j] ;
        if ( j != i )
        {
            t = x [i] ;
            x [i] = x [j] ;
            x [j] = t ;

            t = g [i] ;
            g [i] = g [j] ;
            g [j] = t ;

            t = pg [i] ;
            pg [i] = pg [j] ;
            pg [j] = t ;

            t = lo [i] ;
            lo [i] = lo [j] ;
            lo [j] = t ;

            t = hi [i] ;
            hi [i] = hi [j] ;
            hi [j] = t ;
        }
    }
}

/* =========================================================================
   === asa_shrink_all ======================================================
   =========================================================================
   Shrink vectors x, g, lo and hi from the full space (dimension n)
   to the reduced space (dimension nfree).
   ========================================================================= */

void asa_shrink_all
(
    asa_com *Com
)
{
    int i, j, nfree, *ifree ;
    double t, *lo, *hi, *g, *x ;
    x = Com->x ;
    g = Com->g ;
    lo = Com->lo ;
    hi = Com->hi ;
    ifree = Com->ifree ;
    nfree = Com->nfree ;
    for (j = 0; j < nfree; j++)
    {
        i = ifree [j] ;
        if ( i != j )
        {
            t = x [i] ;
            x [i] = x [j] ;
            x [j] = t ;

            t = g [i] ;
            g [i] = g [j] ;
            g [j] = t ;

            t = lo [i] ;
            lo [i] = lo [j] ;
            lo [j] = t ;

            t = hi [i] ;
            hi [i] = hi [j] ;
            hi [j] = t ;
        }
    }
}

/* =========================================================================
   === asa_dot =============================================================
   =========================================================================
   Compute dot product of x and y, vectors of length n
   ========================================================================= */
double asa_dot
(
    double *x, /* first vector */
    double *y, /* second vector */
    int     n  /* length of vectors */
)
{
    int i, n5 ;
    double t ;
    t = ZERO ;
    n5 = n % 5 ;
    for (i = 0; i < n5; i++) t += x [i]*y [i] ;
    for (; i < n; i += 5)
    {
        t += x [i]*y[i] + x [i+1]*y [i+1] + x [i+2]*y [i+2]
                        + x [i+3]*y [i+3] + x [i+4]*y [i+4] ;
    }
    return (t) ;
}

/* =========================================================================
   === asa_copy ============================================================
   =========================================================================
   Copy vector x into vector y
   ========================================================================= */
void asa_copy
(
    double *y, /* target vector */
    double *x, /* given vector */
    int     n  /* dimension */
)
{
    int j, n10 ;
    n10 = n % 10 ;
    for (j = 0; j < n10; j++) y [j] = x [j] ;
    for (; j < n; j += 10)
    {
        y [j] = x [j] ;
        y [j+1] = x [j+1] ;
        y [j+2] = x [j+2] ;
        y [j+3] = x [j+3] ;
        y [j+4] = x [j+4] ;
        y [j+5] = x [j+5] ;
        y [j+6] = x [j+6] ;
        y [j+7] = x [j+7] ;
        y [j+8] = x [j+8] ;
        y [j+9] = x [j+9] ;
    }
}


/* =========================================================================
   === asa_saxpy ===========================================================
   =========================================================================
   Compute z = y + ax
   ========================================================================= */
void asa_saxpy
(
    double *z,
    double *y,
    double *x,
    double  a,
    int     n
)
{
    int j, n5 ;
    n5 = n % 5 ;
    for (j = 0; j < n5; j++) z [j] = y [j] + a*x [j] ;
    for (; j < n; j += 5)
    {
        z [j]   = y [j]   + a*x [j] ;
        z [j+1] = y [j+1] + a*x [j+1] ;
        z [j+2] = y [j+2] + a*x [j+2] ;
        z [j+3] = y [j+3] + a*x [j+3] ;
        z [j+4] = y [j+4] + a*x [j+4] ;
    }
}
/* =========================================================================
   === asa_max =============================================================
   =========================================================================
   Return max {fabs (x [j]) : 1 <= j < n}
   ========================================================================= */
double asa_max
(
    double *x,
    int     n
)
{
    double xnorm ;
    int j, n5 ;
    n5 = n % 5 ;
    xnorm = ZERO ;
    for (j = 0; j < n5; j++) if ( xnorm < fabs (x [j]) ) xnorm = fabs (x [j]) ;
    for (; j < n; j += 5)
    {
        if ( xnorm < fabs (x [j]  ) ) xnorm = fabs (x [j]) ;
        if ( xnorm < fabs (x [j+1]) ) xnorm = fabs (x [j+1]) ;
        if ( xnorm < fabs (x [j+2]) ) xnorm = fabs (x [j+2]) ;
        if ( xnorm < fabs (x [j+3]) ) xnorm = fabs (x [j+3]) ;
        if ( xnorm < fabs (x [j+4]) ) xnorm = fabs (x [j+4]) ;
    }
    return (xnorm) ;
}

/* =========================================================================
   === asa_printcgParms ====================================================
   =========================================================================
   Print the contents of the asacg_parm structure
   ========================================================================= */
void asa_printcgParms
(
    asacg_parm  *Parm
)
{
    printf ("\nCG PARAMETERS:\n") ;
    printf ("\n") ;
    printf ("Wolfe line search parameter ..................... delta: %e\n",
             Parm->delta) ;
    printf ("Wolfe line search parameter ..................... sigma: %e\n",
             Parm->sigma) ;
    printf ("decay factor for bracketing interval ............ gamma: %e\n",
             Parm->gamma) ;
    printf ("growth factor for bracket interval ................ rho: %e\n",
             Parm->rho) ;
    printf ("growth factor for bracket interval after nan .. nan_rho: %e\n",
             Parm->nan_rho) ;
    printf ("decay factor for stepsize after nan ......... nan_decay: %e\n",
             Parm->nan_decay) ;
    printf ("parameter in lower bound for beta ........... BetaLower: %e\n",
             Parm->BetaLower) ;
    printf ("parameter describing cg_descent family .......... theta: %e\n",
             Parm->theta) ;
    printf ("perturbation parameter for function value ......... eps: %e\n",
             Parm->eps) ;
    printf ("factor by which eps grows if necessary .......... egrow: %e\n",
             Parm->egrow) ;
    printf ("factor for computing average cost .............. Qdecay: %e\n",
             Parm->Qdecay) ;
    printf ("relative change in cost to stop QuadStep ... QuadCutOff: %e\n",
             Parm->QuadCutOff) ;
    printf ("cost change factor, approx Wolfe transition . AWolfeFac: %e\n",
             Parm->AWolfeFac) ;
    printf ("restart cg every restart_fac*n iterations . restart_fac: %e\n",
             Parm->restart_fac) ;
    printf ("cost error in quadratic restart is qeps*cost ..... qeps: %e\n",
             Parm->qeps) ;
    printf ("number of quadratic iterations before restart  qrestart: %i\n",
             Parm->qrestart) ;
    printf ("parameter used to decide if cost is quadratic ... qrule: %e\n",
             Parm->qrule) ;
    printf ("stop when cost change <= feps*|f| ................. eps: %e\n",
             Parm->feps) ;
    printf ("starting guess parameter in first iteration ...... psi0: %e\n",
             Parm->psi0) ;
    printf ("factor multiply starting guess in quad step ...... psi1: %e\n",
             Parm->psi1) ;
    printf ("initial guess factor for general iteration ....... psi2: %e\n",
             Parm->psi2) ;
    printf ("starting step in first iteration if nonzero ...... step: %e\n",
             Parm->step) ;
    printf ("max tries to find non NAN function value ...... nshrink: %i\n",
             Parm->nshrink) ;
    printf ("max expansions in line search ................. nexpand: %i\n",
             Parm->nexpand) ;
    printf ("max secant iterations in line search .......... nsecant: %i\n",
             Parm->nsecant) ;
    printf ("max cg iterations is n*maxit_fac ............ maxit_fac: %e\n",
             Parm->maxit_fac) ;
    printf ("total max cg iterations is n*totit_fac ...... totit_fac: %e\n",
             Parm->totit_fac) ;
    printf ("error tolerance when debugger turned on ..... .debugtol: %e\n",
             Parm->debugtol) ;
    printf ("print level (0 = none, 4 = maximum) ........ PrintLevel: %i\n",
             Parm->PrintLevel) ;
    printf ("\nLogical parameters:\n") ;
    if ( Parm->PertRule )
        printf ("    Error estimate for function value is eps*Ck\n") ;
    else
        printf ("    Error estimate for function value is eps\n") ;
    if ( Parm->QuadStep )
        printf ("    Use quadratic interpolation step\n") ;
    else
        printf ("    No quadratic interpolation step\n") ;
    if ( Parm->AdaptiveBeta )
        printf ("    Adaptively adjust direction update parameter beta\n") ;
    else
        printf ("    Use fixed parameter theta in direction update\n") ;
    if ( Parm->PrintParms )
        printf ("    Print the parameter structure\n") ;
    else
        printf ("    Do not print parameter structure\n") ;
    if ( Parm->AWolfe)
        printf ("    Approximate Wolfe line search\n") ;
    else
        printf ("    Wolfe line search") ;
        if ( Parm->AWolfeFac > ZERO )
            printf (" ... switching to approximate Wolfe\n") ;
        else
            printf ("\n") ;
    if ( Parm->debug)
        printf ("    Check for decay of cost, debugger is on\n") ;
    else
        printf ("    Do not check for decay of cost, debugger is off\n") ;
}

/* =========================================================================
   === asa_default ======================================================
   =========================================================================
   Set default parameter values for the ASA routine. The CG default
   parameter values are set by asa_cg_default.  If the parameter argument of
   asa_descent is NULL, this routine is called by asa_cg automatically.
   If the user wishes to set parameter values, then the asa_parameter structure
   should be allocated in the main program. The user could call asa_default
   to initialize the structure, and then individual elements in the structure
   could be changed, before passing the structure to asa_cg.
   =========================================================================*/
void asa_default
(
    asa_parm *Parm
)
{
    double eps, t ;

    /* T => print final statistics
       F => no printout of statistics */
    Parm->PrintFinal = TRUE ;

    /* Level 0  = no printing), ... , Level 4 = maximum printing */
    Parm->PrintLevel = 0 ;

    /* T => print parameters values
       F => do not display parameter values */
    Parm->PrintParms = FALSE ;

    /* T => use approximate nonmonotone Armijo line search
       F => use ordinary nonmonotone Armijo line search, switch to
            approximate Armijo when |f_r-f| < AArmijoFac*|min (f_r, f_{max})| */
    Parm->AArmijo = FALSE ;
    Parm->AArmijoFac = 1.e-8 ;

    /* Stop Rules (these override the corresponding cg parameters):
       T => ||proj_grad||_infty <= max(grad_tol,initial ||grad||_infty*StopFac)
       F => ||proj_grad||_infty <= grad_tol*(1 + |f_k|) */
    Parm->StopRule = TRUE ;
    Parm->StopFac = 0.e-12 ;

    /* T => estimated error in function value = eps*|min (f_r, f_{max}) |
       F => estimated error in function value = eps */
    Parm->PertRule = TRUE ;
    Parm->eps = 1.e-6 ;

    /* T => only use gradient projection algorithm
       F => let algorithm decide between grad_proj and cg_descent */
    Parm->GradProjOnly = FALSE ;

    /* maximum number of times the Armijo line search will perform
       backtracking steps */
    Parm->max_backsteps = (int) 50 ;

    /* abort cbb after maxit_fac*n iterations in one pass through cbb */
    Parm->maxit_fac = INF ;

    /* abort cbb after totit_fac*n iterations in all passes through cbb */
    Parm->totit_fac = INF ;

    /* abort cbb iteration after maxfunc_fac*n function evaluations */
    Parm->maxfunc_fac = INF ;

    /* perturbation in bounds based on machine epsilon, which we now compute */
    eps = ONE ;
    t = ONE ;
    while ( t > 0 )
    {
        eps /= TWO ;
        t = ONE + eps ;
        t -= ONE ;
    }
    eps *= 2 ;                   /* machine epsilon */
    Parm->pert_lo = 1.e3*eps ;   /* perturbation of lower bounds */
    Parm->pert_hi = 1.e3*eps ;   /* perturbation of upper bounds */

    /* search for non nan function value by shrinking search interval
       at most nshrink times */
    Parm->nshrink = (int) 50 ;

    /* factor by which interval shrinks when searching for non nan value */
    Parm->nan_fac = 2.e-1 ;

    /* update fr if fmin was not improved after L iterations */
    Parm->L = 3 ;

    /* fmax = max (f_{k-i}, i = 0, 1, ..., min (k, m-1) ) */
    Parm->m = 8 ;

    /* update fr if initial stepsize was accepted in previous P iterations */
    Parm->P = 40 ;

    /* CBB cycle length */
    Parm->nm = 4 ;

    /* Reinitialize BB stepsize, if (s^t y)/(||s|| ||y||) >= gamma
       and ||s|| <= min (parm3*|f_k+1|/||g_k+1||_infty, 1) */
    Parm->gamma = 0.975e0 ;

    /* update reference value fr if (fr-fmin)/(fc-fmin) > gamma1 */
    Parm->gamma1 = (double) Parm->m / (double) Parm->L ;

    /* update fr if (fr-f)/(fmax-f) > gamma2, np > P, and fmax > f */
    Parm->gamma2 = (double) Parm->P / (double) Parm->m ;

    /* terminate Armijo line search when
       phi(alpha) <= phi_r + alpha * delta * phi'(0) where phi_r = fr or fcomp*/
    Parm->delta = 1.0e-4 ;   /* Armijo line search parameter */

    /* stepsize s in the line search must satisfy lmin <= s <= lmax */
    Parm->lmin = 1.0e-20 ;
    Parm->lmax = 1.0e+20 ;

    /* attempt a quadratic interpolation step in cg_descent if the
       provisional stepsize times parm1 <= stepsize to boundary */
    Parm->parm1 = 1.e-1 ;

    /* if quadratic interpolation step is attempted, the provisional step
       is at most parm2*stepsize to boundary */
    Parm->parm2 = 9.e-1 ;

    /* used in the the criterion of reinitializing the BB stepsize */
    Parm->parm3 = 1.e-1 ;

    /* maximum number of previous BB steps used when s^t y <= ZERO */
    Parm->parm4 = 6 ;

    /* if ginorm < tau1*pgnorm, continue gradient projection steps  */
    Parm->tau1 = 1.e-1 ;

    /* decay factor for tau1 */
    Parm->tau1_decay = 5.e-1 ;

    /* ginorm < tau2*pgnorm implies subproblem solved in cgdescent */
    Parm->tau2 = 1.e-1 ;

    /* decay factor for tau2 */
    Parm->tau2_decay = 5.e-1 ;

    /* if pgnorm < pgdecay*MAX (pgnorm0, ONE), check the undecided index set
                                pgnorm0 = pgnorm at starting point */
    Parm->pgdecay = 1.e-4 ;

    /* backtracking decay factor in the Armijo line search */
    Parm->armijo_decay = 5.e-1 ;

    /* use quadratic interpolation to compute Armijo step if it
       lies in the interval [.1 alpha, .9 alpha] */
    Parm->armijo0 = 1.e-1 ;
    Parm->armijo1 = 9.e-1 ;
}

/* =========================================================================
   === asa_cg_default ======================================================
   =========================================================================
   Set default conjugate gradient parameter values. If the parameter argument
   of asa_cg is NULL, this routine is called by asa_cg automatically.
   If the user wishes to set parameter values, then the asa_parameter structure
   should be allocated in the main program. The user could call asa_cg_default
   to initialize the structure, and then individual elements in the structure
   could be changed, before passing the structure to asa_cg.
   =========================================================================*/
void asa_cg_default
(
    asacg_parm   *Parm
)
{
    /* Level 0 = no printing, ... , Level 4 = maximum printing */
    Parm->PrintLevel = 0 ;

    /* T => print parameters values
       F => do not display parameter values */
    Parm->PrintParms = FALSE ;

    /* T => use approximate Wolfe line search
       F => use ordinary Wolfe line search, switch to approximate Wolfe when
                |f_k+1-f_k| < AWolfeFac*C_k, C_k = average size of cost */
    Parm->AWolfe = FALSE ;
    Parm->AWolfeFac = 1.e-3 ;

    /* T => estimated error in function value is eps*Ck,
       F => estimated error in function value is eps */
    Parm->PertRule = TRUE ;
    Parm->eps = 1.e-6 ;

    /* factor by which eps grows when line search fails during contraction */
    Parm->egrow = 10. ;

    /* T => attempt quadratic interpolation in line search when
                |f_k+1 - f_k|/f_k <= QuadCutOff
       F => no quadratic interpolation step */
    Parm->QuadStep = TRUE ;
    Parm->QuadCutOff = 1.e-12 ;

    /* T => check that f_k+1 - f_k <= debugtol*C_k
       F => no checking of function values */
    Parm->debug = FALSE ;
    Parm->debugtol = 1.e-10 ;

    /* factor in [0, 1] used to compute average cost magnitude C_k as follows:
       Q_k = 1 + (Qdecay)Q_k-1, Q_0 = 0,  C_k = C_k-1 + (|f_k| - C_k-1)/Q_k */
    Parm->Qdecay = 0.7 ;

    /* if step is nonzero, it is the initial step of the initial line search */
    Parm->step = ZERO ;

    /* abort cg after maxit_fac*n iterations in one pass */
    Parm->maxit_fac = INF ;

    /* abort cg after totit_fac*n iterations in all passes */
    Parm->totit_fac = INF ;

    /* maximum number of times the bracketing interval grows */
    Parm->nexpand = (int) 50 ;

    /* maximum number of times the bracketing interval shrinks */
    Parm->nshrink = (int) 6 ;

    /* maximum number of secant iterations in line search is nsecant */
    Parm->nsecant = (int) 50 ;

    /* conjugate gradient method restarts after (n*restart_fac) iterations */
    Parm->restart_fac = 6.0 ;

    /* stop when -alpha*dphi0 (estimated change in function value) <= feps*|f|*/
    Parm->feps = ZERO ;

    /* after encountering nan, growth factor when searching for
       a bracketing interval */
    Parm->nan_rho = 1.3 ;

    /* after encountering nan, decay factor for stepsize */
    Parm->nan_decay = 0.1 ;

    /* Wolfe line search parameter, range [0, .5]
       phi (a) - phi (0) <= delta phi'(0) */
    Parm->delta = .1 ;

    /* Wolfe line search parameter, range [delta, 1]
       phi' (a) >= sigma phi' (0) */
    Parm->sigma = .9 ;

    /* decay factor for bracket interval width in line search, range (0, 1) */
    Parm->gamma = .66 ;

    /* growth factor in search for initial bracket interval */
    Parm->rho = 5. ;

    /* starting guess for line search =
         psi0 ||x_0||_infty over ||g_0||_infty if x_0 != 0
         psi0 |f(x_0)|/||g_0||_2               otherwise */
    Parm->psi0 = .01 ;      /* factor used in starting guess for iteration 1 */

    /* for a QuadStep, function evaluated at psi1*previous step */
    Parm->psi1 = .1 ;

    /* when starting a new cg iteration, our initial guess for the line
       search stepsize is psi2*previous step */
    Parm->psi2 = 2. ;

    /* choose theta adaptively if AdaptiveBeta = T */
    Parm->AdaptiveBeta = FALSE ;

    /* lower bound for beta is BetaLower*d_k'g_k/ ||d_k||^2 */
    Parm->BetaLower = 0.4 ;

    /* value of the parameter theta in the cg_descent update formula:
       W. W. Hager and H. Zhang, A survey of nonlinear conjugate gradient
       methods, Pacific Journal of Optimization, 2 (2006), pp. 35-58. */
    Parm->theta = 1.0 ;

    /* parameter used in cost error estimate for quadratic restart criterion */
    Parm->qeps = 1.e-12 ;

    /* number of iterations the function is nearly quadratic before a restart */
    Parm->qrestart = 3 ;

    /* treat cost as quadratic if
       |1 - (cost change)/(quadratic cost change)| <= qrule */
    Parm->qrule = 1.e-8 ;
}

/* =========================================================================
   === asa_printParms ====================================================
   =========================================================================
   Print the contents of the asa_parm structure
   ========================================================================= */
void asa_printParms
(
    asa_parm  *Parm
)
{
    printf ("\nASA PARAMETERS:\n") ;
    printf ("\n") ;
    printf ("update fr if fmin not improved after L iterations.... L: %i\n",
             Parm->L) ;
    printf ("fmax = max (f_{k-i}, i = 0, 1, ..., min (k, m-1) )... m: %i\n",
             Parm->m) ;
    printf ("update fr if P previous initial stepsizes accepted... P: %i\n",
             Parm->P) ;
    printf ("CBB cycle length.................................... nm: %i\n",
             Parm->nm) ;
    printf ("criterion for updating reference value fr....... gamma1: %e\n",
             Parm->gamma1) ;
    printf ("criterion for updating reference value fr....... gamma2: %e\n",
             Parm->gamma2) ;
    printf ("interval decay factor in NAN search ............nan_fac: %e\n",
             Parm->nan_fac) ;
    printf ("perturbation parameter for function value.......... eps: %e\n",
             Parm->eps) ;
    printf ("cost change factor, approx Armijo transition,AArmijoFac: %e\n",
             Parm->AArmijoFac) ;
    printf ("Armijo line search parameter .................... delta: %e\n",
             Parm->delta) ;
    printf ("Armijo decay factor .......................armijo_decay: %e\n",
             Parm->armijo_decay) ;
    printf ("criterion for Q interpolation, cbb line search,.armijo0: %e\n",
             Parm->armijo0) ;
    printf ("criterion for Q interpolation, cbb line search,.armijo1: %e\n",
             Parm->armijo1) ;
    printf ("criterion for reinitializing BB stepsize ........ gamma: %e\n",
             Parm->gamma) ;
    printf ("Lower bound for initial stepsize ................. lmin: %e\n",
             Parm->lmin) ;
    printf ("Upper bound for initial stepsize ................. lmax: %e\n",
             Parm->lmax) ;
    printf ("used when trying a quadratic interpolation step.. parm1: %e\n",
             Parm->parm1) ;
    printf ("used when trying a quadratic interpolation step.. parm2: %e\n",
             Parm->parm2) ;
    printf ("criterion for reinitializing the BB stepsize..... parm3: %e\n",
             Parm->parm3) ;
    printf ("maximum previous BB steps used when s^t y <= 0... parm4: %i\n",
             Parm->parm4) ;
    printf ("if ginorm < tau1*pgnorm, continue grad_proj ...... tau1: %e\n",
             Parm->tau1) ;
    printf ("decay factor for tau1 ...................... tau1_decay: %e\n",
             Parm->tau1_decay) ;
    printf ("ginorm < tau2*pgnorm => subproblem solved in cg... tau2: %e\n",
             Parm->tau2) ;
    printf ("decay factor for tau2 ...................... tau2_decay: %e\n",
             Parm->tau2_decay) ;
    printf ("max number of Armijo backtracking steps . max_backsteps: %i\n",
             Parm->max_backsteps) ;
    printf ("max cbb iterations in 1 pass is n*maxit_fac . maxit_fac: %e\n",
             Parm->maxit_fac) ;
    printf ("max number of contracts in the line search .... nshrink: %i\n",
             Parm->nshrink) ;
    printf ("total number cbb iterations is n*totit_fac .. totit_fac: %e\n",
             Parm->totit_fac) ;
    printf ("max func evals in cbb is n*maxfunc_fac .... maxfunc_fac: %e\n",
             Parm->maxfunc_fac) ;
    printf ("criterion for checking undecided index set..... pgdecay: %e\n",
             Parm->pgdecay) ;
    printf ("perturbation of lower bounds .................. pert_lo: %e\n",
             Parm->pert_lo) ;
    printf ("perturbation of upper bounds .................. pert_hi: %e\n",
             Parm->pert_hi) ;
    printf ("factor multiplying gradient in stop condition . StopFac: %e\n",
             Parm->StopFac) ;
    printf ("print level (0 = none, 4 = maximum) ........ PrintLevel: %i\n",
             Parm->PrintLevel) ;
    printf ("\nLogical parameters:\n") ;
    if ( Parm->PertRule )
        printf ("    Error estimate for function value is eps*|fcomp|\n") ;
    else
        printf ("    Error estimate for function value is eps\n") ;
    if ( Parm->PrintFinal )
        printf ("    Print final cost and statistics\n") ;
    else
        printf ("    Do not print final cost and statistics\n") ;
    if ( Parm->PrintParms )
        printf ("    Print the parameter structure\n") ;
    else
        printf ("    Do not print parameter structure\n") ;
    if ( Parm->AArmijo)
        printf ("    Approximate nonmonotone Armijo line search\n") ;
    else
        printf ("    Nonmonotone Armijo line search") ;
        if ( Parm->AArmijoFac > ZERO )
            printf (" ... switching to approx nonmonotone Armijo\n") ;
        else
            printf ("\n") ;
    if ( Parm->StopRule )
    {
        if ( Parm->StopFac == ZERO )
        {
            printf ("    Stopping condition based on gradient tolerance\n") ;
        }
        else
        {
            printf ("    Stopping condition uses initial grad tolerance\n") ;
        }
    }
    else
        printf ("    Stopping condition weighted by absolute cost\n") ;
    if ( Parm->GradProjOnly )
        printf ("    Only use the gradient projection algorithm\n") ;
    else
        printf ("    Apply gradient projection algorithm and cg_descent\n") ;
}
/*
Version 1.1 Change:
    1. Pass a structure asa_objective to the user evaluation routines.
       This allows asa_cg to pass more information to the user which
       might be used to speedup his routines to evaluate the objective
       function and its gradient.  Two elements of the structure are
       ifree and nfree.  If ifree is not NULL, then ifree is a pointer
       to an integer array containing the indices of the free variables
       while nfree is the number of free variables.

    2. Halt the Armijo backtracking line search in cbb when the number
       of backtracking steps reaching Parm->max_backsteps

Version 1.2:
    Correct the Armijo line search in cbb by dividing by including the
    factor "/alpha" in the termination condition

Version 1.3:
    In asa_identify, correct the formula for identifying constraints.

Version 2.0:
    Update the cg routine utilizing cg_descent 4.0

Version 2.1:
    Correct several bugs connected with installation of cg_descent 4.0

Version 2.2:
    1. Modify cg_descent line search so that when there are too many
       contractions, the code will increase eps and switch to expansion
       of the search interval.  This fixes some cases where the code
       terminates when eps is too small.  When the estimated error in
       the cost function is too small, the algorithm could fail in
       cases where the slope is negative at both ends of the search
       interval and the objective function value on the right side of the
       interval is larger than the value at the left side (because the true
       objective function value on the right is not greater than the value on
       the left).
    2. Fix bug in asa_lineW
*/

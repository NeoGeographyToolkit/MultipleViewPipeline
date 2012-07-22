/*=================================================================
 *
 * asa_wrapper.c, mex gatewate interface
 *
 * Written by Stephen Becker, March 22 2012
 *
 *
 * Inputs (in order):
 *  x       initial guess or current point, also used to determine the size of the problem (Nx1)
 *  l       list of upper bounds (Nx1)
 *  u       list of lower bounds (Nx1)
 *  f       value of function (output is 1x1)
 *  g       gradient of function (output is Nx1)
 *  fg      (optional) function that computes f and g (output is [1x1, Nx1] )
 *  opts    (optional) structure of options; see asa_user.h
 *  opts_CG (optional) structure of options for CG subsolver; see asa_user.h
 *  f_param (optional) structure that will be passed to the Matlab functions f, g and fg
 *
 *  Warning: this does not assume any default values for x, l and u.
 *
 *  when compiling the ASA code, add in -DMEXPRINTF'
 *  and it will call "mexPrintf" instead of "printf",
 *  so you can see results on the Matlab command prompt.
 *
 *  compile with:  mex -DMEXPRINTF asa_wrapper.c -IASA_CG-2.2/ ASA_CG-2.2/asa_cg.c -largeArrayDims
 *
 *=================================================================*/

/* The order of the inputs */
#define N_x 0
#define N_l 1
#define N_u 2
#define N_f 3
#define N_g 4
#define N_fg 5  
#define N_opts 6
#define N_optsCG 7
#define N_param 8

#include <math.h>
#include "mex.h"
#include <string.h> /* for strcpy */

/* Declare the ASA function */
#include "asa_user.h"

#ifdef DEBUG
#define debugPrintf mexPrintf
#else
#define debugPrintf fakePrintf
#endif
/* If the 'DEBUG' symbol is undefined, then don't print: */
int fakePrintf(const char *format, ...){
    return 0;
}


/* some global variables */
char *valFcn, *gradFcn, *valGradFcn;
/* Allow the user to pass in an extra struct 
 * that will be passed to their val, grad and valGrad
 * fucntions.
 * e.g. param=struct('A',A,'b',b); */
mxArray *f_param;

/* prototypes for the function and gradient evaluation routines */
double myvalue( asa_objective *asa );
void mygrad( asa_objective *asa );
double myvalgrad( asa_objective *asa );

/* some helper functions */
void parseOptions_CG( asacg_parm *cgParm, const mxArray *opts );
void parseOptions( asa_parm *asaParm, const mxArray *opts );
void errorString( int errInt );
void copyStatToStruct(mxArray *Struct, asa_stat Stat );

/* Main mex gateway routine */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray*prhs[] )   { 
    
    int     i, n, rValue;
    double  *x, *lo, *hi, *out;
    int     USE_FCN_GRAD = 0;
    asa_stat Stat;
    asacg_parm cgParm ;
    asa_parm   asaParm ;    
    
    /* Parse inputs. Quite boring */
    
    if (nrhs < 5 ) mexErrMsgTxt("Needs at least 5 input arguments");
    if ( mxGetN(prhs[N_x]) != 1 ) mexErrMsgTxt("x must be a column vector");
    n = (int) mxGetM( prhs[N_x] );
    if ( mxGetM(prhs[N_l]) != n ) mexErrMsgTxt("l must have same size as x");
    if ( mxGetM(prhs[N_u]) != n ) mexErrMsgTxt("u must have same size as x");
    x       = mxGetPr( prhs[N_x] );
    lo      = mxGetPr( prhs[N_l] );
    hi      = mxGetPr( prhs[N_u] );

    valFcn = mxArrayToString( prhs[N_f] );
    if (valFcn==NULL) mexErrMsgTxt("Must supply a string for the fcn and grad arguments");

    gradFcn = mxArrayToString( prhs[N_g] );
    if (gradFcn ==NULL) mexErrMsgTxt("Must supply a string for the fcn and grad arguments");

    if (nrhs > 5 ) {
        valGradFcn = mxArrayToString( prhs[N_fg] );
        if (valGradFcn!=NULL){
            USE_FCN_GRAD = 1;
            mexPrintf("Using combined fcn_grad function '%s'\n", valGradFcn);
        }
    }

    /* copy the "x" input into the "out" variable, and then operate
     * on "out" in-place. this way Matlab users won't be surprised
     * to have their variables modified in-place */
    plhs[0] = mxCreateDoubleMatrix(n,1, mxREAL);
    out     = (double *)mxGetPr( plhs[0] );
    for (i = 0; i < n; i++) out[i] = x[i];

    /* Some default parameters; user can change these in Matlab via the options structures */
    asa_cg_default( &cgParm ) ;
    asa_default(    &asaParm ) ;
    cgParm.PrintParms  = TRUE ;
    cgParm.PrintLevel  = 0 ;
    asaParm.PrintParms = TRUE ;
    asaParm.PrintLevel = 0 ;


    /* pass in options */
    if ( nrhs > 6 )
        parseOptions( &asaParm, prhs[N_opts] );
    if ( nrhs > 7 )
        parseOptions_CG( &cgParm, prhs[N_optsCG] );
    if ( nrhs > 8 ){
        /* careful, this should be const */
        f_param = (mxArray *)prhs[N_param];
        if (!mxIsStruct( f_param) )
            f_param = NULL;
    } else {
        f_param = NULL;
    }


    /* -- Finally, done with parsing inputs. Now, call ASA C routine */
    if (USE_FCN_GRAD==1)
        rValue = asa_cg(out, lo, hi, n, &Stat, &cgParm, &asaParm, 1.e-8, myvalue, mygrad, myvalgrad, NULL) ; 
    else
        rValue = asa_cg(out, lo, hi, n, &Stat, &cgParm, &asaParm, 1.e-8, myvalue, mygrad, NULL , NULL) ;
            
    mxFree(valFcn );
    mxFree(gradFcn);
    mxFree(valGradFcn );


    /* don't need to free the memory for "x" if I return it in a variable,
     * since then Matlab will handle it automatically */
            
    if (nlhs > 1 ){
        plhs[1] = mxCreateDoubleScalar( (double)rValue );
    }
    errorString( rValue );

    if ( nlhs > 2 ) {
        const char *fieldNames[] = {"f",
                "pgnorm","cbbiter","cbbfunc","cbbgrad",
                "cgiter","cgfunc","cggrad" };
        plhs[2] = mxCreateStructMatrix(1,1, 8, fieldNames );
        copyStatToStruct( plhs[2], Stat );
    
    }
    return;
}

/* evaluate the objective function */
/* (this will call a Matlab function to do so) */
double myvalue ( asa_objective *asa ) {
    INT i, n ;
    double f, *x, *xx;
    mxArray *lhs[1], *rhs[2];  /* might not need all 2 entries... */
    x = asa->x ;
    n = asa->n ;

    /* Call Matlab */
    rhs[0] = mxCreateDoubleMatrix( n, 1, mxREAL ); 
    xx = mxGetPr( rhs[0] ); 
    for (i = 0; i < n; i++) 
        xx[i] = x[i]; 
    if (f_param==NULL)
        mexCallMATLAB( 1, lhs, 1, rhs, valFcn );  
    else{
        rhs[1] = f_param; /* pass in extra options */
        mexCallMATLAB( 1, lhs, 2, rhs, valFcn );  
    }
    f = *mxGetPr( lhs[0] ); 
    mxDestroyArray( rhs[0] ); 
    mxDestroyArray( lhs[0] ); 
    return (f) ;
}

/* evaluate the gradient of the objective function */
void mygrad ( asa_objective *asa ) {
    INT i, n ;
    double *g, *x, *gg, *xx; 
    mxArray *lhs[1], *rhs[2];  /* might not need all 2 entries... */

    x = asa->x ;
    g = asa->g ;
    n = asa->n ;

    /* Call Matlab */
    rhs[0] = mxCreateDoubleMatrix( n, 1, mxREAL ); 
    xx = mxGetPr( rhs[0] ); 
    for (i = 0; i < n; i++) 
        xx[i] = x[i]; 
    if (f_param==NULL)
        mexCallMATLAB( 1, lhs, 1, rhs, gradFcn );  
    else{
        rhs[1] = f_param; /* pass in extra options */
        mexCallMATLAB( 1, lhs, 2, rhs, gradFcn );  
    }
    if (mxGetNumberOfElements( lhs[0] )!= n ) mexErrMsgTxt("The gradient is the wrong size\n");
    gg = mxGetPr( lhs[0] ); 
    for (i = 0; i < n; i++) 
        g[i] = gg[i]; 
    mxDestroyArray( rhs[0] ); 
    mxDestroyArray( lhs[0] ); 
    return ;
}

/* value and gradient of the objective function */
double myvalgrad( asa_objective *asa ) { 
    INT i, n ; 
    double f, *g, *x, *gg, *xx; 
    mxArray *lhs[2], *rhs[2]; 
    x = asa->x ; 
    g = asa->g ; 
    n = asa->n ; 

    rhs[0] = mxCreateDoubleMatrix( n, 1, mxREAL ); 
    xx = mxGetPr( rhs[0] ); 
    for (i = 0; i < n; i++) 
        xx[i] = x[i]; 
    if (f_param==NULL)
        mexCallMATLAB( 2, lhs, 1, rhs, valGradFcn );  
    else{
        rhs[1] = f_param; /* pass in extra options */
        mexCallMATLAB( 2, lhs, 2, rhs, valGradFcn );  
    }
    if (mxGetNumberOfElements( lhs[1] )!= n ) {
        debugPrintf("Size of lhs[1] is %d x %d\n", mxGetM(lhs[1]), mxGetN(lhs[1]) );

        mexErrMsgTxt("The gradient is the wrong size\n");
    }
    if (mxGetNumberOfElements( lhs[0] )!= 1 ) mexErrMsgTxt("The function is the wrong size\n");
    f = *mxGetPr( lhs[0] ); 
    gg = mxGetPr( lhs[1] ); 
    for (i = 0; i < n; i++) 
        g[i] = gg[i]; 
    mxDestroyArray( rhs[0] ); 
    mxDestroyArray( lhs[0] ); 
    mxDestroyArray( lhs[1] ); 
    return (f) ; 
} 

/* 
 * These next functions are based off the typedef's in asa_user.h
 * If you use a version of ASA other than v2.2, you may need
 * to modify these a little bit
 * */

void parseOptions_CG( asacg_parm *cgParm, const mxArray *opts ){
    int n, i;
    const char **fnames;       /* pointers to field names */

    double value;
    if (mxIsStruct( opts ) ) {
        n = mxGetNumberOfFields(opts);
        fnames = mxCalloc(n, sizeof(*fnames));
        for (i=0; i<n; i++ ){
            fnames[i] = mxGetFieldNameByNumber( opts, i );
            if (fnames[i]==NULL) mexErrMsgTxt("failure parsing options");
            value = *mxGetPr( mxGetFieldByNumber( opts, 0, i ) );

            debugPrintf("For CG, fieldname %s has value %f\n", fnames[i], value );

            /* see asa_user.h for a description */
            if (strcmp(fnames[i], "PrintParms") == 0) 
                cgParm->PrintParms = (int)value;
            else if (strcmp(fnames[i], "PrintLevel") == 0) 
                cgParm->PrintLevel = (int)value;
            else if (strcmp(fnames[i], "AWolfe") == 0) 
                cgParm->AWolfe = (int)value;
            else if (strcmp(fnames[i], "AWolfeFac") == 0) 
                cgParm->AWolfeFac = (double)value;
            else if (strcmp(fnames[i], "Qdecay") == 0) 
                cgParm->Qdecay = (int)value;
            else if (strcmp(fnames[i], "PertRule") == 0) 
                cgParm->PertRule = (int)value;
            else if (strcmp(fnames[i], "eps") == 0) 
                cgParm->eps = (double)value;
            else if (strcmp(fnames[i], "egrow") == 0) 
                cgParm->egrow = (double)value;
            else if (strcmp(fnames[i], "QuadStep") == 0) 
                cgParm->QuadStep = (int)value;
            else if (strcmp(fnames[i], "QuadCutOff") == 0) 
                cgParm->QuadCutOff = (double)value;
            else if (strcmp(fnames[i], "debug") == 0) 
                cgParm->debug = (int)value;
            else if (strcmp(fnames[i], "debugtol") == 0) 
                cgParm->debugtol = (double)value;
            else if (strcmp(fnames[i], "step") == 0) 
                cgParm->step = (double)value;
            else if (strcmp(fnames[i], "maxit_fac") == 0) 
                cgParm->maxit_fac = (double)value;
            else if (strcmp(fnames[i], "totit_fac") == 0) 
                cgParm->totit_fac = (double)value;
            else if (strcmp(fnames[i], "nexpand") == 0) 
                cgParm->nexpand = (int)value;
            else if (strcmp(fnames[i], "nshrink") == 0) 
                cgParm->nshrink = (int)value;
            else if (strcmp(fnames[i], "nsecant") == 0) 
                cgParm->nsecant = (int)value;
            else if (strcmp(fnames[i], "restart_fac") == 0) 
                cgParm->restart_fac = (double)value;
            else if (strcmp(fnames[i], "feps") == 0) 
                cgParm->feps = (double)value;
            else if (strcmp(fnames[i], "nan_rho") == 0) 
                cgParm->nan_rho = (double)value;
            else if (strcmp(fnames[i], "nan_decay") == 0) 
                cgParm->nan_decay = (double)value;
            /* not allowing user to change the "technical" parameters */

        }
    }
}
void parseOptions( asa_parm *asaParm, const mxArray *opts ){
    int n, i;
    const char **fnames;       /* pointers to field names */

    double value;
    if (mxIsStruct( opts ) ) {
        n = mxGetNumberOfFields(opts);
        fnames = mxCalloc(n, sizeof(*fnames));
        for (i=0; i<n; i++ ){
            fnames[i] = mxGetFieldNameByNumber( opts, i );
            if (fnames[i]==NULL) mexErrMsgTxt("failure parsing options");
            value = *mxGetPr( mxGetFieldByNumber( opts, 0, i ) );

            debugPrintf("Fieldname %s has value %f\n", fnames[i], value );

            /* see asa_user.h for a description */
            if (strcmp(fnames[i], "PrintFinal") == 0) 
                asaParm->PrintFinal = (int)value;
            else if (strcmp(fnames[i], "PrintLevel") == 0) 
                asaParm->PrintLevel = (int)value;
            else if (strcmp(fnames[i], "PrintParms") == 0) 
                asaParm->PrintParms = (int)value;
            else if (strcmp(fnames[i], "AArmijo") == 0) 
                asaParm->AArmijo = (int)value;
            else if (strcmp(fnames[i], "AArmijoFac") == 0) 
                asaParm->AArmijoFac = (double)value;
            else if (strcmp(fnames[i], "StopRule") == 0) 
                asaParm->StopRule = (int)value;
            else if (strcmp(fnames[i], "StopFac") == 0) 
                asaParm->StopFac = (double)value;
            else if (strcmp(fnames[i], "PertRule") == 0) 
                asaParm->PertRule = (int)value;
            else if (strcmp(fnames[i], "eps") == 0) 
                asaParm->eps = (double)value;
            else if (strcmp(fnames[i], "GradProjOnly") == 0) 
                asaParm->GradProjOnly = (int)value;
            else if (strcmp(fnames[i], "max_backsteps") == 0) 
                asaParm->max_backsteps = (int)value;
            else if (strcmp(fnames[i], "maxit_fac") == 0) 
                asaParm->maxit_fac = (double)value;
            else if (strcmp(fnames[i], "totit_fac") == 0) 
                asaParm->totit_fac = (double)value;
            else if (strcmp(fnames[i], "maxfunc_fac") == 0) 
                asaParm->maxfunc_fac = (double)value;
            else if (strcmp(fnames[i], "pert_lo") == 0) 
                asaParm->pert_lo = (double)value;
            else if (strcmp(fnames[i], "pert_hi") == 0) 
                asaParm->pert_hi = (double)value;
            else if (strcmp(fnames[i], "nshrink") == 0) 
                asaParm->nshrink = (int)value;
            else if (strcmp(fnames[i], "nan_fac") == 0) 
                asaParm->nan_fac = (double)value;
            /* not allowing user to change the "technical" parameters */

        }
    }
}

void errorString( int errInt ) {
    char errString[80]; 
    switch (errInt ) {
		case -2: strcpy(errString,"function value became nan in cg"); break;
		case -1: strcpy(errString,"starting function value is nan in cg"); break;
		case 0: strcpy(errString,"convergence tolerance satisfied"); break;
		case 1: strcpy(errString,"change in func <= feps*|f| in cg"); break;
		case 2: strcpy(errString,"cg iterations in all passes or in one pass exceeded their limit"); break;
		case 3: strcpy(errString,"slope always negative in line search in cg"); break;
		case 4: strcpy(errString,"number secant iterations exceed nsecant in cg"); break;
		case 5: strcpy(errString,"search direction not a descent direction in cg"); break;
		case 6: strcpy(errString,"line search fails in initial interval in cg"); break;
		case 7: strcpy(errString,"line search fails during bisection in cg"); break;
		case 8: strcpy(errString,"line search fails during interval update in cg"); break;
		case 9: strcpy(errString,"debugger is on and the function value increases in cg"); break;
		case 10: strcpy(errString,"out of memory"); break;
		case 11: strcpy(errString,"cbb iterations in all passes or in one pass exceeded their limit"); break;
		case 12: strcpy(errString,"line search failed in cbb iteration"); break;
		case 13: strcpy(errString,"search direction in cbb is not descent direction"); break;
		case 14: strcpy(errString,"function value became nan in cbb"); break; 
        default: strcpy(errString,"Unrecognized error code!"); break;
    }
    mexPrintf("%s", errString );
}

void copyStatToStruct(mxArray *Struct, asa_stat Stat ) {
    mxArray *f0, *f1, *f2, *f3, *f4, *f5, *f6, *f7;

    f0 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f0) = Stat.f;
    mxSetFieldByNumber( Struct, 0, 0, f0 );

    f1 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f1) = Stat.pgnorm;
    mxSetFieldByNumber( Struct, 0, 1, f1 );

    f2 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f2) = (int)Stat.cbbiter;
    mxSetFieldByNumber( Struct, 0, 2, f2 );

    f3 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f3) = (int)Stat.cbbfunc;
    mxSetFieldByNumber( Struct, 0, 3, f3 );

    f4 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f4) = (int)Stat.cbbgrad;
    mxSetFieldByNumber( Struct, 0, 4, f4 );

    f5 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f5) = (int)Stat.cgiter;
    mxSetFieldByNumber( Struct, 0, 5, f5 );

    f6 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f6) = (int)Stat.cgfunc;
    mxSetFieldByNumber( Struct, 0, 6, f6 );

    f7 = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(f7) = (int)Stat.cggrad;
    mxSetFieldByNumber( Struct, 0, 7, f7 );

}

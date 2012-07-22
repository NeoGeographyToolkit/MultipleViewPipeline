%{
Stephen Becker, March 27 2012

Installation and sample usage of the asa_wrapper mex file "asa_wrapper.c"
We show how to use the helper files asa_fcn, asa_grad and asa_fcnGrad

Info about ASA is here:
    http://www.math.ufl.edu/~hager/papers/CG/
W. W. Hager and H. Zhang, A new active set algorithm for box constrained optimization, SIAM Journal on Optimization, 17 (2006), pp. 526-557
http://www.math.ufl.edu/~hager/papers/CG/asa.pdf

source code for ASA version 2.2 (April 14, 2011) is here:
http://www.math.ufl.edu/~hager/papers/CG/Archive/ASA_CG-2.2.tar.gz

This file will download ASA, install it, and compile
the mex files. We also show a simple example
(the same example as driver1.c in the ASA package, except
now in Matlab), as well as a non-negative least-squares
problem example.

%}

% download the code
url ='http://www.math.ufl.edu/~hager/papers/CG/Archive/ASA_CG-2.2.tar.gz';
url ='../ASA_CG-2.2.tar';
srcDir = untar(url,'./');

if isunix
    % This will modify the ASA .c file to allow it
    % to print to the Matlab screen (it replaces
    % "printf" with "mexPrintf")
    unix('./modify_printf.sh');
else
    % If you don't have linux/unix, just edit asa_cg.c
    %   by hand and do a search/replace with a text
    %   editor, and add these lines to the top of the file:
%{
#ifdef MEXPRINTF
#define Printf mexPrintf
#else
#define Printf printf
#endif
%}
    
    % This isn't necessary, but it's nice to have output
    % on the screen.
end

% compile with
mex -DMEXPRINTF asa_wrapper.c ASA_CG-2.2/asa_cg.c -IASA_CG-2.2 -largeArrayDims -v

%% Run a sample problem
n = 5000000;
lo = zeros(n,1);    % lower bound
hi = ones(n,1);     % upper bound
x  = 0.5*ones(n,1);     % initial guess; necessary so that it knows the size of the problem

% To use this on your own, make these strings point
%   to your function. The "fcn" is a function that computes
%   your objective function; "grad" computes its gradient.
%   The "fcnGrad" is optional; if provided,
%   it computes both the objective and the gradient
%   (since sometimes this saves computation over computing
%    them separately). Its outputs should be [f,g] (f=objective,
%    g=gradient) in that order).

fcn  = 'asa_fcnGrad';
grad = 'asa_grad'; 
fcnGrad = 'asa_fcnGrad'; % optional

% add some ASA options (these are optional). See driver1.c for examples,
%   and see asa_user.h for all possible values
opts = [];
opts.PrintParms = true;
CGopts = struct('PrintParms',true);
param = [];

% run the function
[out,status] = asa_wrapper( x, lo, hi,fcn,grad, fcnGrad, opts, CGopts, param);

%{
Your display should look like this

Using combined fcn_grad function 'asa_fcnGrad'

Final convergence status = 0
Convergence tolerance for gradient satisfied
projected gradient max norm:  3.560360e-09
function value:              -4.011035e+02

Total cg  iterations:                   11
Total cg  function evaluations:         17
Total cg  gradient evaluations:         16
Total cbb iterations:                    3
Total cbb function evaluations:          4
Total cbb gradient evaluations:          4
------------------------------------------
Total function evaluations:             21
Total gradient evaluations:             20
==========================================

%}



%% Test on a non-negative least-squares (NNLS) problem
%{
The NNLS problem:

min_x  .5*||Ax-b||^2  = .5x'A'Ax - x'A'b + .5b'b
subject to
 x >= 0

%}
n = 100;
lo = zeros(n,1);    % lower bound
hi = inf(n,1);     % upper bound
x  = ones(n,1);     % initial guess; necessary so that it knows the size of the problem
A = randn(n,n); %A = A*A'; % make it pos def.
b = randn(n,1);
%% ... first, verify via lsqnonneg
% lsqnonneg is very slow for large problems, but we can
% use it to help verify that we don't have any huge bugs:
x_lsq = lsqnonneg( A, b );
%% ... second, run with ASA
% I have created these 3 special functions just for quadratic problems.
% You don't need to modify every time you change your quadratic
% parameters; rather, pass in the parameters via the "param" structure.
fcn  = 'asa_quadratic_fcn';
grad = 'asa_quadratic_grad'; 
fcnGrad = 'asa_quadratic_fcnGrad'; % optional
param = struct('A',A,'b',b);
% an alternative way:
% param = struct('Q',A'*A,'c',-A'*b,'offset',norm(b)^2/2);


% add some options (these are optional). See driver1.c for examples,
%   and see asa_user.h for all possible values
[PrintParms,CGopts] = deal(struct('PrintParms',false));

% zero-out the counters
asa_quadratic_fcnGrad();


% run the function
[out,status,statistics] = asa_wrapper( x, lo, hi,fcn,grad, fcnGrad, opts, CGopts, param);
fprintf('Error compared to LSQNONNEG version is %.2e\n', ...
    norm( out - x_lsq) );

% View the function values
fcnHistory = asa_quadratic_fcnGrad();
semilogy( fcnHistory - min(fcnHistory), 'o-' );
xlabel('call to the objective function');
ylabel('value of objective function (minus true answer)');
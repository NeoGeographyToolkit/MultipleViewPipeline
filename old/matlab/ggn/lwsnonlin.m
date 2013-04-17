function [X0,Resnorm,FVAL,EXITFLAG,OUTPUT,LAMBDA,JACOB] = lwsnonlin(WFUN,X0,options,varargin)
%LWSNONLIN solves non-linear least squares problems.
%   LWSNONLIN attempts to solve problems of the form:
%   min  sum {WFUN(X).^2}    where X and the values returned by WFUN can be
%    X                      vectors or matrices.
%
%   LWSNONLIN implements two different algorithms: trust region reflective and
%   Levenberg-Marquardt. Choose one via the option Algorithm: for instance, to
%   choose Levenberg-Marquardt, set OPTIONS = optimset('Algorithm','levenberg-marquardt'),
%   and then pass OPTIONS to LWSNONLIN.
%
%   X = LWSNONLIN(WFUN,X0) starts at the matrix X0 and finds a minimum X to
%   the sum of squares of the functions in WFUN. WFUN accepts input X
%   and returns a vector (or matrix) of function values F evaluated
%   at X. NOTE: WFUN should return WFUN(X) and not the sum-of-squares
%   sum(WFUN(X).^2)). (WFUN(X) is summed and squared implicitly in the
%   algorithm.)
%
%   X = LWSNONLIN(WFUN,X0,LB,UB) defines a set of lower and upper bounds on
%   the design variables, X, so that the solution is in the range LB <= X
%   <= UB. Use empty matrices for LB and UB if no bounds exist. Set LB(i)
%   = -Inf if X(i) is unbounded below; set UB(i) = Inf if X(i) is
%   unbounded above.
%
%   X = LWSNONLIN(WFUN,X0,LB,UB,OPTIONS) minimizes with the default
%   optimization parameters replaced by values in the structure OPTIONS,
%   an argument created with the OPTIMSET function. See OPTIMSET for details.
%   Use the Jacobian option to specify that WFUN also returns a second output
%   argument J that is the Jacobian matrix at the point X. If WFUN returns a
%   vector F of m components when X has length n, then J is an m-by-n matrix
%   where J(i,Gk) is the partial derivative of F(i) with respect to x(Gk).
%   (Note that the Jacobian J is the transpose of the gradient of F.)
%
%   X = LWSNONLIN(PROBLEM) solves the non-linear least squares problem
%   defined in PROBLEM. PROBLEM is a structure with the function WFUN in
%   PROBLEM.objective, the start point in PROBLEM.x0, the lower bounds in
%   PROBLEM.lb, the upper bounds in PROBLEM.ub, the options structure in
%   PROBLEM.options, and solver name 'lwsnonlin' in PROBLEM.solver. Use
%   this syntax to solve at the command line a problem exported from
%   OPTIMTOOL. The structure PROBLEM must have all the fields.
%
%   [X,RESNORM] = LWSNONLIN(WFUN,X0,...) returns
%   the value of the squared 2-norm of the residual at X: sum(WFUN(X).^2).
%
%   [X,RESNORM,RESIDUAL] = LWSNONLIN(WFUN,X0,...) returns the value of the
%   residual at the solution X: RESIDUAL = WFUN(X).
%
%   [X,RESNORM,RESIDUAL,EXITFLAG] = LWSNONLIN(WFUN,X0,...) returns an
%   EXITFLAG that describes the exit condition of LWSNONLIN. Possible
%   values of EXITFLAG and the corresponding exit conditions are listed
%   below. See the documentation for a complete description.
%
%     1  LWSNONLIN converged to a solution.
%     2  Change in X too small.
%     3  Change in RESNORM too small.
%     4  Computed search direction too small.
%     0  Too many function evaluations or iterations.
%    -1  Stopped by output/plot function.
%    -2  Bounds are inconsistent.
%
%   [X,RESNORM,RESIDUAL,EXITFLAG,OUTPUT] = LWSNONLIN(WFUN,X0,...) returns a
%   structure OUTPUT with the number of iterations taken in
%   OUTPUT.iterations, the number of function evaluations in
%   OUTPUT.funcCount, the algorithm used in OUTPUT.algorithm, the number
%   of CG iterations (if used) in OUTPUT.cgiterations, the first-order
%   optimality (if used) in OUTPUT.firstorderopt, and the exit message in
%   OUTPUT.message.
%
%   [X,RESNORM,RESIDUAL,EXITFLAG,OUTPUT,LAMBDA] = LWSNONLIN(WFUN,X0,...)
%   returns the set of Lagrangian multipliers, LAMBDA, at the solution:
%   LAMBDA.lower for LB and LAMBDA.upper for UB.
%
%   [X,RESNORM,RESIDUAL,EXITFLAG,OUTPUT,LAMBDA,JACOBIAN] = LWSNONLIN(WFUN,
%   X0,...) returns the Jacobian of WFUN at X.
%
%   Examples
%     WFUN can be specified using @:
%        x = lwsnonlin(@myfun,[2 3 4])
%
%   where myfun is a MATLAB function such as:
%
%       function F = myfun(x)
%       F = sin(x);
%
%   WFUN can also be an anonymous function:
%
%       x = lwsnonlin(@(x) sin(3*x),[1 4])
%
%   If WFUN is parameterized, you can use anonymous functions to capture the
%   problem-dependent parameters. Suppose you want to solve the non-linear
%   least squares problem given in the function myfun, which is
%   parameterized by its second argument c. Here myfun is a MATLAB file
%   function such as
%
%       function F = myfun(x,c)
%       F = [ 2*x(1) - exp(c*x(1))
%             -x(1) - exp(c*x(2))
%             x(1) - x(2) ];
%
%   To solve the least squares problem for a specific value of c, first
%   assign the value to c. Then create a one-argument anonymous function
%   that captures that value of c and calls myfun with two arguments.
%   Finally, pass this anonymous function to LWSNONLIN:
%
%       c = -1; % define parameter first
%       x = lwsnonlin(@(x) myfun(x,c),[1;1])
%
%   See also OPTIMSET, LSQCURVEFIT, FSOLVE, @, INLINE.

%   Copyright 1990-2011 The MathWorks, Inc.
%   $Revision: 1.1.6.16 $  $Date: 2011/07/31 13:12:16 $

% ------------Initialization----------------
defaultopt = struct(...
    'Algorithm','trust-region-reflective',...
    'DerivativeCheck','off',...
    'Diagnostics','off',...
    'DiffMaxChange',Inf,...
    'DiffMinChange',0,...
    'Display','final',...
    'FinDiffRelStep', [], ...
    'FinDiffType','forward',...
    'FunValCheck','off',...
    'Jacobian','off',...
    'JacobMult',[],...
    'JacobPattern','sparse(ones(Jrows,Jcols))',...
    'MaxFunEvals',[],...
    'MaxIter',400,...
    'MaxPCGIter','max(1,floor(numberOfVariables/2))',...
    'OutputFcn',[],...
    'PlotFcns',[],...
    'PrecondBandWidth',Inf,...
    'ScaleProblem','none',...
    'TolFun',1e-6,...
    'TolPCG',0.1,...
    'TolX',1e-6,...
    'TypicalX','ones(numberOfVariables,1)');

% If just 'defaults' passed in, return the default options in X
if nargin==1 && nargout <= 1 && isequal(WFUN,'defaults')
    X0 = defaultopt;
    return
end

[F0,G,Fk,Gk,Wk] = feval(WFUN,X0);
[z,s] = elevate(X0,Fk,Gk);
for k = 1:100
    r = 2*Gk'*Wk;
    H = Gk'*diag(Wk./z)*Gk;
    b = -H\r;
    Xk = X0+b;
    [F,G,Fk,Gk,Wk] = feval(WFUN,Xk);
    dF = F-F0;
    if dF < -defaultopt.TolFun,
        [z,s] = elevate(Xk,Fk,Gk,X0,F0);
        F0 = F; X0 = Xk;
    else
        if norm(b) < defaultopt.TolX || dF < defaultopt.TolFun, break; end

        options = optimset('Display','iter'); %,'TolFun',1e-2,'TolX',1e-2);
        [Q,D] = eig(H); d = diag(D);
        [Xk,F,exitflag,output] = fluxSearch(WFUN,X0,Q,d,b,options);
        [F,G,Fk,Gk,Wk] = feval(WFUN,Xk);
    end
end
OUTPUT.iterations = k;
end

function [Z,s] = elevate(Xk,Fk,Gk,X0,F0)
if nargin < 4, X0 = Xk; F0 = Fk; end
% Z is positive
dF = F0-Fk; 
c = Gk.*(X0-Xk)/2;
c2 = c.^2-Fk.*dF;
a = (c2+Fk.*c)./(dF-c);
idx = find(a < -Fk);

% Z is negative
a(idx) = (c2(idx)-Fk(idx).*c(idx))./(dF(idx)+c(idx));
a(isnan(a)) = 0;

Z = Fk+a;
s = sign(Z);
sdx = find(s == 0);
s(sdx) = 1;
Z = Z + s.*realmin;
end
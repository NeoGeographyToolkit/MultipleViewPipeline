function [f,g] = asa_quadratic_fcnGrad(x,varargin)
% For use with the ASA mex wrapper. This computes
%   the quadratic objective and gradient:
%
%   f(x) = 1/2||Ax-b||^2, so gradient = A'*(Ax-b)
%
% if param.A and param.b are supplied,
% and 
%
%   f(x) = 1/2 x'*Q*x + c'*x + offset, so gradient = Qx + c
%
% if param.Q and param.c and param.offset are supplied
%   (and param.Q should be symmetric; if should also
%    be positive semidefinite if you want to get
%    a global solution to your problem).
%
% Stephen Becker, March 23 2012  stephen.beckr@gmail.com

% Optional: record some values of the function
%   To view this value (and reset), call the function
%   with no arguments
global functionHistory 
global nCalls
if nargin == 0
    f = functionHistory(1:nCalls);
    functionHistory = [];
    nCalls = [];
    return;
end
if isempty(functionHistory)
    functionHistory = zeros(100,1);
    nCalls = 0;
end



if nargin < 1, error('Need the "param" structure'); end
param = varargin{1};
if isfield(param,'A')
    Ax = param.A*x;
    if isfield(param,'b'), Ax = Ax - param.b; end
    f = norm(Ax)^2/2;
    % and gradient
    g = (param.A)'*Ax;
elseif isfield(param,'Q')
    g = param.Q*x;
    f = x'*g/2;
    if isfield(param,'c'), f = f + param.c'*x; g = g + param.c; end
    if isfield(param,'offset'), f = f+param.offset; end
else
    error('bad values for param structure');
end

% and update the persistent/global memory
nCalls = nCalls + 1;
N      = length( functionHistory );
if nCalls > N
    functionHistory( N:(2*N) ) = 0;
end
functionHistory(nCalls) = f;
function f = asa_quadratic_fcn(x,varargin)
% For use with the ASA mex wrapper. This computes
%   the quadratic objective:
%
%   f(x) = 1/2||Ax-b||^2
%
% if param.A and param.b are supplied (b default to zero),
% and 
%
%   f(x) = 1/2 x'*Q*x + c'*x + offset
%
% if param.Q and param.c param.offset are supplied
% (c and offset default to zero)
% param.Q should be symmetric, but this is the user's responsibility
%
% Stephen Becker, March 23 2012  stephen.beckr@gmail.com


% Optional: record some values of the function
%   To view this value (and reset), call the function
%   with no arguments
% (I would use 'persistent', but we want to share these
%  between fcn and fcnGrad )
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
elseif isfield(param,'Q')
    Qx = param.Q*x;
    f = x'*Qx/2;
    if isfield(param,'c'), f = f + param.c'*x; end
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
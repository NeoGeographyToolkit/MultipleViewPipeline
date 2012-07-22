function [g] = asa_quadratic_grad(x,varargin)
% For use with the ASA mex wrapper. This computes
%   the quadratic objective and gradient:
%
%   f(x) = 1/2||Ax-b||^2, so gradient = A'*(Ax-b)
%
% if param.A and param.b are supplied,
% and 
%
%   f(x) = 1/2 x'*Q*x + c'*x, so gradient = Qx + c
%
% if param.Q and param.c are supplied
%   (and param.Q should be symmetric; if should also
%    be positive semidefinite if you want to get
%    a global solution to your problem).
%
% Stephen Becker, March 23 2012  stephen.beckr@gmail.com

if nargin < 1, error('Need the "param" structure'); end
param = varargin{1};
if isfield(param,'A')
    Ax = param.A*x;
    if isfield(param,'b'), Ax = Ax - param.b; end
%     f = norm(Ax)^2/2;
    % and gradient
    g = (param.A)'*Ax;
elseif isfield(param,'Q')
    g = param.Q*x;
    if isfield(param,'c'), g = g + param.c; end
else
    error('bad values for param structure');
end
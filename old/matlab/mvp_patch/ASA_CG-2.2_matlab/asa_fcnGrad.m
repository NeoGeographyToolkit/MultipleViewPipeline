function [f,g] = asa_fcnGrad(x,varargin)
% objective and gradient of objective function,
%   for the problem defined in driver1.c for ASA v2.2

e = exp(x);
t = sqrt(1:length(x));
f = sum(e)-t*x;
g = e-t';

if nargin > 1
    opts = varargin{1};
    % Use this to pass in extra information
%     disp(opts)
end
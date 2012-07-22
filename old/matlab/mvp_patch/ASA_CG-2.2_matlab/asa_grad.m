function g = asa_grad(x,varargin)
% gradient function for the problem defined in driver1.c for ASA v2.2

g = exp(x)-sqrt(1:length(x))';

if nargin > 1
    opts = varargin{1};
    % Use this to pass in extra information
%     disp(opts)
end
function f = asa_fcn(x,varargin)

% This is just an example; you can make your own
% Using the example from driver1.c, and
%   not bothering to vectorize it.

f = 0;
% for i = 1:length(x)
%     t = sqrt(i);
%     f = f + exp(x(i)) - t*x(i);
% end
% 
% if nargin > 1
%     opts = varargin{1};
%     % Use this to pass in extra information
% %     disp(opts)
% end
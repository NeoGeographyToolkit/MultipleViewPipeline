function g = mvGrad(w,varargin)
% objective and gradient of objective function,
%   for the problem defined in driver1.c for ASA v2.2
pv = varargin{1}.pv;
pv.W = reshape(w,size(pv.Ws));
pv.proj;
f=reallog(pv.corelate+PatchViews.eps_p);
if nargin > 1
    g = pv.grad_p/pv.p;
end
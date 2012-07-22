function t = SlopyRefine(pv)
Gs = pv.Ws.*pv.Gs;  % smooth image
Gx = pv.Ws.*pv.Gx; Gy = pv.Ws.*pv.Gy; Gt = pv.Ws.*pv.Gt; % weighted gradients
Nx = pv.Wn.*pv.Gx; Ny = pv.Wn.*pv.Gy; Nt = pv.Wn.*pv.Gt; % normalized gradients
%  symmetric components of Hessian
Exx = scatw(Nx(:,:,pv.is).*Gx(:,:,pv.js),Gx.*pv.Gx,pv.ks,pv.x0,pv.y0);
Eyy = scatw(Ny(:,:,pv.is).*Gy(:,:,pv.js),Gy.*pv.Gy,pv.ks,pv.x0,pv.y0);
Ett = scatw(Nt(:,:,pv.is).*Gt(:,:,pv.js),Gt.*pv.Gt,pv.ks,pv.x0,pv.y0);
% asymmetric components of Hessian
Exy = scatw(Nx(:,:,pv.ia).*Gy(:,:,pv.ja),Gx.*pv.Gy,pv.ka,pv.x0,pv.y0);
Ext = scatw(Nx(:,:,pv.ia).*Gt(:,:,pv.ja),Gx.*pv.Gt,pv.ka,pv.x0,pv.y0);
Eyt = scatw(Ny(:,:,pv.ia).*Gt(:,:,pv.ja),Gy.*pv.Gt,pv.ka,pv.x0,pv.y0);
% gradient components
Gx = scatw(Nx(:,:,pv.ia).*Gs(:,:,pv.ja),Gx.*pv.Gs,pv.ka,pv.x0,pv.y0);
Gy = scatw(Ny(:,:,pv.ia).*Gs(:,:,pv.ja),Gy.*pv.Gs,pv.ka,pv.x0,pv.y0);
Gt = scatw(Nt(:,:,pv.ia).*Gs(:,:,pv.ja),Gt.*pv.Gs,pv.ka,pv.x0,pv.y0);

H = [Exx Exy Ext; Exy' Eyy Eyt; Ext' Eyt' Ett];
f = sum([Gx; Gy; Gt],2); w = wnd3(pv.y0,pv.x0,pv.Ws)';    % individual weights

z = zeros(1,pv.n);
A = [w z z; z w z; z z w];
b = zeros(3,1); t0 = zeros(3*pv.n,1);

options = optimset('Display','off');
[t,fval,exitflag] = quadprog(H,f,[],[],A,b,[],[],t0,options);
t = reshape(t,pv.n,3);
end

function [f0 f1 f2]=fncGaussian(r,s)
% r: radius, s: sigma
x=[-r-0.5:r+0.5]'; fx = normpdf(x,0,s);
f0 = diff(normcdf(x,0,s));
f1 = diff(fx);
f2 = diff(-x.*fx)/s^2;
end

function [w0 w1 w2]=wndGaussian(r,s)
% r: radius, s: sigma
[w0 w1 w2]=fncGaussian(r,s);
n0 = sum(w0);
w1 = -w1*s^2/n0;
w2 = s^2*(s^2*w2+w0)/n0;
w0 = w0/n0;
end

function [S,R] = scatw(I,C,K,gx,gy)
% asymmetric scatter
c = wnd3(gy,gx,I);
t = wnd3(gy,gx,C);
R = c(K);
S = diag(t)-R;
end

function b = wnd3(hcol,hrow,A,sz)
% 3-dimensional separable windowing
% b = hcol'*reshape(A,sz(1),prod(sz(2:end)));
% b = hrow'*reshape(b,sz(2),sz(end));
C = hcol*hrow'; if nargin < 4, sz = size(A); end
b = (C(:)'*reshape(A,sz(1)*sz(2),prod(sz(3:end))))';
end

function stop=mvOutFcn(x,optimvalues,state)
if optimvalues.fval < PatchViews.eps_log2p,
    stop = true;
else
    stop = false;
end
end

function [p,f] = fpval(ss,se,ns,ne,eps_p)
if nargin<5, eps_p = realmin; end
if se > 0
    if ss > 0, x = se/(se+ss); else x = 1; end
else
    if ss > 0, x = 0; else x = 0.5; end
end
if ne < eps_p, ne=eps_p; end
if ns < eps_p, ns=eps_p; end
p = betainc(x,ne,ns);
if nargout > 1, f = ss*ne/(se*ns); end
end

function [p,p1,p2] = fbval(ss,se,ns,ne,eps_p)
if nargin<5, eps_p = realmin; end
p1 = fpval(ss,se,ns,ne,eps_p);
p2 = fpval(ns^2*se,ne^2*ss,ns,ne,eps_p);
p = abs(p1-p2);
end
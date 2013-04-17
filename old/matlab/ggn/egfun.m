function [f,g,d,F,G] = egfun(x)
% f = exp(f1+f2+f3)
% f1 =   x1^2+1
% f2 = 2*x2^2+2
% f3 =  -x2^3-1
x = x(:);
A = [1 0; 0 2; 0 -1];
b = [1 2 -1]';
F = A*x.^2 + b;
G = 2*A;
f = exp(sum(F));
d = [f f f]';
g = G'*d;
end
function [f,g,d,F,G] = egfun(x)
% exp(F1+F2+F3)
% F1 =   x1^2+1
% F2 = 2*x2^2+2
% F1 =  -x2^3-1
x = x(:);
A = [1 0; 0 2; 0 -1];
b = [1 2 -1]';
F = A*x.^2 + b;
G = 2*A*x;
f = exp(sum(F));
d = [f f f]';
g = f*sum(G);
end

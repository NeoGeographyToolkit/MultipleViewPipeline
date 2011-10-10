function [f, g] = PoissBias(b,a,c,P);
sz=size(P);
D = P+repmat(b',sz(1),1);
La = log(a*c');
Ld = log(D);
f = D.*Ld-D.*La-D;
f = sum(f(:));

if nargout > 1
    g = sum(Ld-La)';
end
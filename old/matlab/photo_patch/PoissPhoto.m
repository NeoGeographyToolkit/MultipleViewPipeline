function [a, b, c] = PoissPhoto(P)
sz=size(P);
idx0 = find(P==0);
b = zeros(3,1);
c = ones(3,1);
a_old = zeros(sz(1),1);
options = optimset('GradObj','on');
for i = 1:10
    D = P+repmat(b',sz(1),1);
    a = sum(D,2)/sum(c);
    c = sum(D,1)'/sum(a);
    P(idx0) = a(idx0)*c(1)-b(1);
    b = fminunc(@PoissBias,b,options,a,c,P);
    norm(a-a_old)
    a_old = a;
end
function [a, b, c] = GaussPhoto(P);
sz=size(P);
idx0 = find(P==0);
b = zeros(3,1);
c = ones(3,1);
a_old = zeros(sz(1),1);
for i = 1:10
    a = ((P-repmat(b',sz(1),1))*c)/(c'*c);
    P(idx0) = a(idx0)*c(1)+b(1);
    d = [ones(sz(1),1) a]\P;
    b = d(1,:)';
    c = d(2,:)';
    norm(a-a_old)
    a_old = a;
end
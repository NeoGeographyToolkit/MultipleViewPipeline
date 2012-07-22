close all
m = 4;
n = 50000;
k = 1000
for i = 1:k
    X = randn(m,n);
    mX = mean(X,1);
    vX = var(X,1,1);
    m2 = mX*mX';
    sv = sum(vX);
    x = sv/(m2+sv);
    p(i)=betainc(x,(m-1)*n,n);
    fprintf('.')
end
imhist(p)

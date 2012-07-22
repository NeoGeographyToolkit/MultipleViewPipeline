function test_suite = testSlopyRefin
close all
initTestSuite;
end

function testDisplay
% display range data and time stamps
strIn = 'AS15_3_3_tiles.mat';
mv = MultiViews(strIn);
mv.proj([128 150]'); % ([64 64]');

s = 0.5;
w = ceil(s*5);
[s0 s1 s2]=fltGaussian(w,s);            % filters
a = 3*([1:mv.pv.n]-mv.pv.n/2-0.5);      % angles
x = [-53:53];                           % image size
[X Y] = meshgrid(x,x);
I = sign(X.*Y);
X = X(w:end-w-1,w:end-w-1); Y = Y(w:end-w-1,w:end-w-1);
mv.pv.Is=[]; mv.pv.It=[]; mv.pv.Ix=[]; mv.pv.Iy=[]; 
for k=1:mv.pv.n
    Ib(:,:,k) = imrotate(I,a(k),'bicubic','crop');
    mv.pv.Is(:,:,k)=conv2(s0,s0,Ib(:,:,k),'valid');
    mv.pv.Ix(:,:,k)=conv2(s0,s1,Ib(:,:,k),'valid');
    mv.pv.Iy(:,:,k)=conv2(s1,s0,Ib(:,:,k),'valid');
    mv.pv.It(:,:,k)=X.*mv.pv.Iy(:,:,k)-Y.*mv.pv.Ix(:,:,k);
end
mv.pv.Ws = ones(size(mv.pv.Is));
mv.pv.Wn = mv.pv.Ws/mv.pv.n;
mv.pv.Gs=mv.pv.Is; mv.pv.Gt=mv.pv.It;
mv.pv.Gx=mv.pv.Ix; mv.pv.Gy=mv.pv.Iy;
mv.pv.disp
t = SlopyRefin(mv.pv)
b = t(:,3)*180/pi;
for k=1:mv.pv.n
    Ib(:,:,k) = imrotate(I,-a(k)-b(k),'bicubic','crop');
    mv.pv.Is(:,:,k)=conv2(s0,s0,Ib(:,:,k),'valid');
    mv.pv.Ix(:,:,k)=conv2(s0,s1,Ib(:,:,k),'valid');
    mv.pv.Iy(:,:,k)=conv2(s1,s0,Ib(:,:,k),'valid');
    mv.pv.It(:,:,k)=X.*mv.pv.Iy(:,:,k)-Y.*mv.pv.Ix(:,:,k);
end
mv.pv.Gs=mv.pv.Is; mv.pv.Gt=mv.pv.It;
mv.pv.Gx=mv.pv.Ix; mv.pv.Gy=mv.pv.Iy;
mv.pv.disp
t = SlopyRefin(mv.pv)
end

function [f0 f1 f2]=fncGaussian(r,s)
    % r: radius, s: sigma
    x=[-r-0.5:r+0.5]'; fx = normpdf(x,0,s);
    f0 = diff(normcdf(x,0,s));
    f1 = diff(fx);
    f2 = diff(-x.*fx)/s^2;
end

function [f0 f1 f2]=fltGaussian(r,s)
    % r: radius, s: sigma
    [f0 f1 f2]=SingleView.fncGaussian(r,s);
    n0 = sum(f0); f0 = f0/n0; f1 = f1/n0; f2 = f2/n0;
end

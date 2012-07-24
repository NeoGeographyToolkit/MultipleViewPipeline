function DriverMultiViews
clc; close all;
radiusMoon = 1737400;
lb = radiusMoon - 3000;
ub = radiusMoon + 3000;
step = 25;

strIn = 'AS15_3_3_tiles.mat';
mv = MultiViews(strIn);
mv.proj([128 150]'); % ([64 64]');
mv.corelate;
r = mv.elevate(radiusMoon-500); h = r-radiusMoon
q = mv.rotate
for k=1:50
    k
    [p(k),~,~,f(k)] = mv.pv.corelate;
    t = mv.pv.geometry
    mv.proj
end
figure, plot(log(f))
figure, plot(log(p))
mv.pv.disp
% c = mv.slote
% mv.corelate;
% mv.pv.disp
t = mv.scate
s = mv.smote
% [p,a,b,t] = mv.corelate;
for i=1:2
    i
    r = mv.elevate; h = r-radiusMoon
    q = mv.rotate
    c = mv.slote
    t = mv.scate
    s = mv.smote
end
mv.profile(lb,ub,step);
mv.profile_scx
% mv.profile(lb,ub,step);
% mv.pv.disp
end
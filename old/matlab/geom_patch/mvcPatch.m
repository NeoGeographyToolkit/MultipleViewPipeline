% read multiple-view data 
% clear all
% Mviews=mvread();
gDEM = imread('../../ground-DEM.tif');
sDEM = imread('../../initial-DEM.tif');

Patch.center=[521; 521];
Patch.width=[32 8]; % correlation and smoothing window size
Patch.height=double(gDEM(Patch.center(2), Patch.center(1))) + 10;
Patch.range=Patch.height+[-20 20];     % crop the region of interest
Patch.fill = 1e6;   % scale of FillValues for imtransform
Patch.plane=[pix2dir(Mviews.georef,Patch.center); ...
    Mviews.radius+Patch.height];

Patch=mpcrop(Mviews,Patch);
Patch=mporthoproj(Patch);

options = optimset('LargeScale','on','Display','iter');
options = optimset(options,'GradObj','on','GradConstr','on');
options = optimset(options,'TolFun',1e-12,'TolX',1e-9);
options = optimset(options,'MaxFunEvals',120000,'MaxIter',150);
options = optimset(options,'MaxTime',600);

before = now;
datestr(before)

% f=[];
% q1 = p(1)-2e-4:5e-6:p(1)+1e-4;
% q2 = p(4)-1e1:5e-1:p(4)+2e1;
% [Q1,Q2]=meshgrid(q1,q2);
% n=length(Q1(:));
% for i=1:n
%     p(1)=Q1(i);
%     p(4)=Q2(i);
%     f = [f mpgeoGaus(p,Patch)];
% end
% F = reshape(f,size(Q1));
% mesh(Q1,Q2,F)

p = Patch.plane;
[p,fval,exitflag,output] = fmincon(@(p)mpgeoGaus(p,Patch),p,[],[],[],[], ...
    p-[0.5*[1;1;1]; 1000],p+[0.5*[1;1;1]; 1000],@(p)UnitNorm(p),options);
%    p-[0.005*[1;1;1]; 100],p+[0.005*[1;1;1]; 100],@(p)UnitNorm(p),options);

after = now;
datestr(before)
datestr(after)
(after-before)*24*3600

oPatch=Patch;
oPatch.plane=p;
oPatch.height=p(4)/(p(1:3)'*pix2dir(Mviews.georef,Patch.center))-Mviews.radius;
oPatch=mporthoproj(oPatch);
for i=1:numel(oPatch.ortho)
     figure(1), subplot(2,2,i), imshow(oPatch.image{i});
     figure(2), subplot(2,2,i), imshow(oPatch.ortho{i});
end
e = pix2dir(Mviews.georef,Patch.center);
p(4)/(p(1:3)'*e)-Mviews.radius
double(gDEM(Patch.center(2), Patch.center(1)))
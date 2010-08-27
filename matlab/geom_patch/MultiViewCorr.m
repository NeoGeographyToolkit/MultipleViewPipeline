% read multiple-view data 
% Mviews=mvread();

Patch.center=[600; 400];
Patch.width=[30 3]; % correlation and smoothing window size
Patch.height=-2605;
Patch.range=[-2605 -2605];     % crop the region of interest
Patch.plane=[pix2dir(Mviews.georef,Patch.center); ...
    Mviews.radius+Patch.height];

Patch=mvcrop(Mviews,Patch);
Patch=mvorthoproj(Patch);

p = Patch.plane;
mvGeoPatchGaus(p,Patch);

options = optimset('LargeScale','on','Display','iter');
options = optimset(options,'GradObj','on','GradConstr','on');
options = optimset(options,'TolFun',1e-7,'TolX',1e-7);
options = optimset(options,'MaxFunEvals',120000,'MaxIter',300);

before = now;
datestr(before)

[p,fval,exitflag,output] = fmincon(@(p)mvGeoPatchGaus(p,Patch),p,[],[],[],[], ...
    [],[],@(p)UnitNorm(p),options);

after = now;
datestr(before)
datestr(after)
(after-before)*24*3600

for i=1:numel(Patch.ortho)
     figure(1), subplot(2,2,i), imshow(Patch.image{i});
     figure(2), subplot(2,2,i), imshow(Patch.ortho{i});
end
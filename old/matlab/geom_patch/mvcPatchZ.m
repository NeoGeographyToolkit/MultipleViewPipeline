% read multiple-view data
Mviews=mvread();
gDEM = imread('../../ground-DEM.tif');
sDEM = imread('../../initial-DEM.tif');
sz = size(sDEM);

itr = 500;
sidx = 48;
eidx = 976;
load '../../rDEM'
[I,J]=find(rDEM(sidx:eidx,sidx:eidx) == 0);
I = I + sidx - 1; J = J + sidx - 1;
idx=sub2ind(sz,I,J);

options = optimset('Display','off');
% options = optimset(options,'GradObj','off','GradConstr','on');
options = optimset(options,'GradObj','off','LargeScale','off');
options = optimset(options,'TolFun',1e-15,'TolX',1e-12);
options = optimset(options,'MaxFunEvals',120000,'MaxIter',150);
options = optimset(options,'MaxTime',600);

before = now;
Patch.fill = 1e6;   % scale of FillValues for imtransform
Patch.width=[32 8]; % correlation and smoothing window size
for i=3:length(idx)
    Patch.center=[J(i); I(i)];
    Patch.height=double(sDEM(Patch.center(2), Patch.center(1)));
    Patch.range=Patch.height+[-1000 1000];     % crop the region of interest
    Patch.plane=[pix2dir(Mviews.georef,Patch.center); ...
        Mviews.radius+Patch.height];

    Patch=mpcrop(Mviews,Patch);
    p = Patch.plane(4);
    [p,fval,exitflag,output] = fminunc(@(p)mpgeoGausP4(p,Patch),p,options);

    rDEM(I(i),J(i)) = p-Mviews.radius;
    fprintf('.');
    if rem(i,itr) == 0
        t = double(gDEM(Patch.center(2), Patch.center(1)));
        str = sprintf('\n (%d,%d) Error: %f = %f - (%f)',I(i),J(i),rDEM(I(i),J(i))-t,rDEM(I(i),J(i)),t);
        disp(str);
        after = now;

        save '../../rDEM' rDEM
        str = sprintf('time: %s = %s - %s \n',(after-before)*24*3600,datestr(after),datestr(before));
        disp(str);
        before = after;
        oPatch=Patch;
        oPatch.plane=[Patch.plane(1:3); p];
        oPatch.height=p-Mviews.radius;
        oPatch=mporthoproj(oPatch);
        for k=1:numel(oPatch.ortho)
            figure(1), subplot(2,2,k), imshow(oPatch.image{k});
            figure(2), subplot(2,2,k), imshow(oPatch.ortho{k});
        end
    end
end
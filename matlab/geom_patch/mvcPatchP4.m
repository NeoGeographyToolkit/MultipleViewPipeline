% read multiple-view data 
Mviews=mvread();
gDEM = imread('../../ground-DEM.tif');
sDEM = imread('../../initial-DEM.tif');

Patch.center=[521; 521];
Patch.width=[32 8]; % correlation and smoothing window size
Patch.height=double(gDEM(Patch.center(2), Patch.center(1)))-20;
Patch.range=Patch.height+[-200 200];     % crop the region of interest
Patch.fill = 1e6;   % scale of FillValues for imtransform
Patch.plane=[pix2dir(Mviews.georef,Patch.center); ...
    Mviews.radius+Patch.height];

Patch=mpcrop(Mviews,Patch);
Patch=mporthoproj(Patch);

F = []; G = [];
Q = Mviews.radius+double(gDEM(Patch.center(2), Patch.center(1))) + [-50:50];
for q = Q
    [f,g]=mpgeoGausP4(q,Patch);
    F = [F f];
    G = [G g];
end
dF = conv(F,[1 0 -1]/2);

close all
figure, plot(Q,F), axis([min(Q) max(Q) min(F) max(F)])
figure, plot(Q,G);
figure, plot(Q,G,'r',Q,dF(2:end-1),'b')
axis([min(Q) max(Q) -1e-5 1e-5])


% Correlator Comparison
close all

max_abs = 5;
range = [50 -50; 80 -80; 50 -50; 50 -50];

gDEM = double(imread('../../ground-DEM.tif'));
load '../../rDEM'
idx=find(rDEM);
DEM.str{2} = '../../stereo-nosubpx-DEM__ground-DEM-diff.tif';
DEM.str{3} = '../../stereo-parabola-DEM__ground-DEM-diff.tif';
DEM.str{4} = '../../stereo-bayes-DEM__ground-DEM-diff.tif';
figure, mesh(gDEM), colorbar, axis equal, view(0,90)
minDEM = min(gDEM(:));
maxDEM = max(gDEM(:));

idx=find(rDEM);
dDEM = zeros(size(rDEM));
dDEM(idx) = rDEM(idx)-gDEM(idx);
idx = find(rDEM ~= 0 & dDEM < range(1,1) & dDEM > range(1,2));
mDEM = mean(dDEM(idx));
sDEM = std(dDEM(idx));
[mDEM sDEM]
% dDEM(find(dDEM>range(1,1)))=range(1,1);
% dDEM(find(dDEM<range(1,2)))=range(1,2);
figure, mesh(tanh(dDEM/max_abs)*max_abs), colorbar, axis equal, view(0,90)

rDEM=gDEM+dDEM;
rDEM(find(rDEM>maxDEM))=maxDEM;
rDEM(find(rDEM<minDEM))=minDEM;
figure, mesh(rDEM), colorbar, axis equal, view(0,90)

for i=2:4
    dDEM = double(imread(DEM.str{i}));
    idx = find(dDEM < range(i,1) & dDEM > range(i,2));
    mDEM = mean(dDEM(idx));
    sDEM = std(dDEM(idx));
    [mDEM sDEM]
    %     dDEM(find(dDEM>range(i,1)))=range(i,1);
    %     dDEM(find(dDEM<range(i,2)))=range(i,2);
    figure, mesh(tanh(dDEM/max_abs)*max_abs), colorbar, axis equal, view(0,90)
    rDEM=gDEM+dDEM;
    rDEM(find(rDEM>maxDEM))=maxDEM;
    rDEM(find(rDEM<minDEM))=minDEM;
    figure, mesh(rDEM), colorbar, axis equal, view(0,90)
end
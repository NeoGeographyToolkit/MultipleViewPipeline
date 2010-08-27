close all

max_abs = 5;
range = [50 -50; 80 -80; 50 -50; 50 -50];

gDEM = imread('../../ground-DEM.tif');
tDEM=load('../../rDEM');
bDEM=load('F:/MultiviewTK/rDEM');

idx=find(bDEM.rDEM);
tDEM.rDEM(idx)=bDEM.rDEM(idx);

rDEM = tDEM.rDEM;

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

% save '../../rDEM' rDEM
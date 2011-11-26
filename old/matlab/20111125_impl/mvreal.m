function Views=mvreal(w)
% read multiple-view data

strpath = '../../real_scene/';

% convert 0-based index to 1-based index
index_base = 1;

% radius of the moon
Views.radius = 1737400;
Views.radiusMoon = 1737400;
% homography to rebase the index
H = [[eye(2) [index_base; index_base]]; zeros(1,2) 1];

load([strpath 'cams'])
Views.camera{1}=cam0;
Views.camera{2}=cam1;
Views.camera{3}=cam2;
Views.camera{4}=cam3;

if nargin > 0
    strDEM0 = [strpath 'ground-DEM.mat'];
    if exist(strDEM0,'file')
        tmp=load(strDEM0);
        Views.DEMinit=tmp.DEMinit;
    else
        Views.DEMinit=imread([strpath 'ground-DEM.tif']);
        DEMinit=mvsmooth(Views.DEMinit,w);
        Views.DEMinit=DEMinit;
        save(strDEM0,'DEMinit');
    end

    strDEM = [strpath 'rDEM.mat'];
    if exist(strDEM,'file')
        tmp=load(strDEM);
        Views.DEM=tmp.rDEM;
    else
        DEM=nan(size(Views.DEMinit));
        Views.DEM=DEM;
        save(strDEM,'DEM');
    end
end

Views.n = numel(Views.camera);
for i=1:Views.n
    % post-rebased camera matrices
    Views.camera{i}=H*Views.camera{i};
    str = sprintf('%s%d.tif',strpath,i-1);
    Views.image{i}=double(imread(str));
    Views.image{i}=Views.image{i}(:,:,1)/2^16;
end

% pre-rebased georeference homography
Views.georef = geo/H;
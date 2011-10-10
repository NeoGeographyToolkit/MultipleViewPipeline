function Views=mvread(index_base)
% read multiple-view data 

% convert 0-based index to 1-based index
if nargin < 1
    index_base = 1;
end

% radius of the moon
Views.radius = 1737400;

% homography to rebase the index 
H = [[eye(2) [index_base; index_base]]; zeros(1,2) 1];

Views.camera{1}=[-4274.71,-260.205,-1930.17,5.08022e+09;
    1144.09,-1103.48,-3549.36,1.61622e+09;
    -0.530706,-0.835777,-0.14081,1.85456e+06];
Views.camera{2}=[-3610.07,840.684,-1762.02,2.83176e+09;
    1130.09,-1093.56,-3558.98,1.61788e+09;
    -0.545648,-0.824206,-0.151499,1.85442e+06];
Views.camera{3}=[-2928.63,1877.03,-1578.87,6.62652e+08;
    1117.75,-1093.48,-3572.36,1.63467e+09;
    -0.556218,-0.814156,-0.166648,1.85437e+06];
Views.camera{4}=[-2207.5,2904.16,-1361.04,-1.54366e+09;
    1121.87,-1072.53,-3577.41,1.60066e+09;
    -0.56798,-0.803401,-0.178733,1.85418e+06];

for i=1:numel(Views.camera)
    % post-rebased camera matrices
    Views.camera{i}=H*Views.camera{i};
    str = sprintf('../../%d.tif',i-1);
    Views.image{i}=double(imread(str));
end

% pre-rebased georeference homography
Views.georef = [0.00133653359715, 0.00000000000000, 55.7209107019;
    0.00000000000000,-0.00133653359715, 10.1949968063;
    0.00000000000000, 0.00000000000000, 1.00000000000]/H;

% homography to convert angular unit from degrees to radians
H = diag([pi/180 pi/180 1]);
Views.georef = H*Views.georef;
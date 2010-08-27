% test mvJacP4 (Spatial Jacobian at the Center of a Patch)
% This test the mvJacP4 at the center point by default

Mviews=mvread();
gDEM = imread('../../ground-DEM.tif');

Patch.center=fix(1023*rand(2,1)+1); % [521; 521];
Patch.center'
Patch.point=Patch.center+1; %fix(1023*rand(2,1)+1);
Patch.width=[32 8]; % correlation and smoothing window size
Patch.height=double(gDEM(Patch.center(2), Patch.center(1)));
Patch.range=Patch.height+[-200 200];     % crop the region of interest
Patch.fill = 1e6;   % scale of FillValues for imtransform
Patch.plane=[pix2dir(Mviews.georef,Patch.center); ...
    Mviews.radius+Patch.height];

Patch=mpcrop(Mviews,Patch);
Patch=mporthoproj(Patch);
J = mvJacP4(Patch.plane(4), Patch);

F = []; G = [];
Q = Mviews.radius+double(gDEM(Patch.center(2), Patch.center(1))) + [-50:50];
for q = Q
    Patch.plane(4)=q;
    Patch=mporthoproj(Patch);
    [J, oPatch] = mvJacP4(Patch.plane(4), Patch);
    Patch.plane(4)=q+1;
    rPatch=mporthoproj(Patch);

    dz = [];
    for i=1:numel(rPatch.camera)
        % correponding point in the orbital image
        
        z=rPatch.ihomo{i}*oPatch.sight{i};
        dz=[dz z(1:2)/z(3)-oPatch.point];
    end
    fprintf('right gradient at height: %f norm(Jac-dz)=%f \n',q, norm(J-dz));

    Patch.plane(4)=q-1;
    lPatch=mporthoproj(Patch);
    dz = [];
    for i=1:numel(rPatch.camera)
        % correponding point in the orbital image
        
        z=lPatch.ihomo{i}*oPatch.sight{i};
        dz=[dz oPatch.point-z(1:2)/z(3)];
    end
    fprintf(' left gradient at height: %f norm(Jac-dz)=%f \n',q, norm(J-dz));
end
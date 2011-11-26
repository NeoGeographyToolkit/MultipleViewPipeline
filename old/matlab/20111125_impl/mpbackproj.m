function [Ortho, R] = mpbackproj(Patch, R)
% Patch: multiple view structure
% Ortho: Back-projected images
% Sight: Projected point of Patch Center
% R: Rotation

if nargin < 2, R=q2dcm(Patch.q); end
for i=1:Patch.n
    % homography
    Sight{i} = Patch.r*Patch.Pe{i}+Patch.camera{i}(:,4);
    H = [Patch.camera{i}(:,1:3)*R(:,1:2)*diag(Patch.scaleSpatial) Sight{i}];

    tform = maketform('projective',inv(H)');
    Ortho(:,:,i) = imtransform(Patch.image{i},tform,'bicubic',...
        'xdata',Patch.xdata,'ydata',Patch.ydata);
end
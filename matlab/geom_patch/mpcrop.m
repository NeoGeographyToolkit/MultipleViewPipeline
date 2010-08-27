function Patch = mpcrop(Views,Patch)

if ~isfield(Patch, 'range')
    Patch.range = [Patch.elevation Patch.elevation];
end

if numel(Views.image) ~= numel(Views.camera)
    disp('error in szR(2) ~= szA(3)');
end

% augment the width by radius (2) of the cubic nterpolation 
w = Patch.width(1)/2 + 2;
Patch.georef=Views.georef;

for i=1:numel(Views.camera)
    z = repmat(Patch.center,1,4)+w*[1 1 -1 -1; 1 -1 1 -1];
    e = pix2dir(Views.georef,z);
    r = Views.radius+Patch.range;
    u = Views.camera{i}*[e*r(1) e*r(2); ones(1,8)];
    b = u(1:2,:)./[u(3,:); u(3,:)];

    maxb = ceil(max(b,[],2));
    minb = floor(min(b,[],2));

    Patch.image{i}=imcrop(Views.image{i},[minb maxb-minb-1]);

    % homography to rebase the index
    H=[eye(2) 1-minb; zeros(1,2) 1];
    Patch.camera{i}=H*Views.camera{i};
end

end %mvcrop
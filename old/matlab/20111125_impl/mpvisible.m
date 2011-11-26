function visible = mpvisible(Views)

if ~isfield(Views, 'rangeLarge')
    Views.rangeLarge = [Views.elevation Views.elevation];
end

if ~isfield(Views, 'radiusMoon')
    Views.radiusMoon = Views.radiusMoon;
end

if numel(Views.image) ~= numel(Views.camera)
    disp('error in szR(2) ~= szA(3)');
end

if ~isfield(Views, 'v')
    v = pix2dir(Views.georef,Views.center);
else
    v = Views.v;
end

% augment the width by radiusMoon (2) of the cubic nterpolation 
Views.georef=Views.georef;

% center normal
n = pix2dir(Views.georef,Views.center);
visible = true;
for i=1:numel(Views.camera)
    z = repmat(Views.center,1,4)+Views.radiusPatch(1)*[1 1 -1 -1; 1 -1 1 -1];
    e = pix2dir(Views.georef,z);
    s = (v'*n)./(v'*e);
    es = e*diag(s);
    r = Views.radiusMoon+Views.rangeLarge;
    u = Views.camera{i}*[es*r(1) es*r(2); ones(1,8)];
    b = u(1:2,:)./[u(3,:); u(3,:)];

    maxb = ceil(max(b,[],2));
    minb = floor(min(b,[],2));

    if minb > 0 & maxb < size(Views.image{i})'
        Views.visible{i} = true;
    else
        Views.visible{i} = false;
        visible = false;
    end
end
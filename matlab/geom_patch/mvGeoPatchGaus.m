function [f,g]=mvgeopatchGaus(p,Patch)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% I: Orbital images
% P: Projection matrices
% Patch.georef: Homgraphy transformation
% Patch.center: center point

p = p/norm(p(1:3)); % normalized plane
v = p(1:3); % normal vector
d = p(4);   % distance from the origin

% correlation and smoothing windows
gc = diff(normcdf([-Patch.width(1)/2-0.5:Patch.width(1)/2+0.5],0,Patch.width(1)/6));
gc = gc/sum(gc);
gs = diff(normcdf([-Patch.width(2)/2-0.5:Patch.width(2)/2+0.5],0,Patch.width(2)/6));
gs = gs/sum(gs);

% direction vector and its Jacobian
[e, de] = pix2dir(Patch.georef,Patch.center);

Patch=mvorthoproj(Patch);
Im = conv2(gs,gs,Patch.ortho{1},'same');
f = gc*Im.^2*gc';
n = numel(Patch.ortho);
for i=2:n
    Is = conv2(gs,gs,Patch.ortho{i},'same');
    f = f + gc*Is.^2*gc';
    Im = Im + Is;
end
f = f - gc*Im.^2*gc' / n;

% Gradient of the objective function
if nargout > 1
    g=mpgrad(p, Patch);
end



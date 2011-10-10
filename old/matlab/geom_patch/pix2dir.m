function [e, d]=pix2dir(H,z)
% compute spherical direction from orthographic image coordinates.

sz = size(z);
% spherical angles from pixel coordinates in a orthographic image
a = H*[z;ones(1,sz(2))];        % spherical angles (longitude and latitude)
a = a(1:2,:)./[a(3,:);a(3,:)];  % inhomonizing the coordinates (radians)
ca = cos(a);        % cosine of spherical angles
sa = sin(a);        % sine of spherical angles

% direction vector and its Jacobian
e = [ca(1,:).*ca(2,:); sa(1,:).*ca(2,:); sa(2,:)];
if nargout == 2
    % longitudinal and latitudianl Jacobians
    dlon = [-e(2,:); e(1,:); zeros(1,sz(2))];
    dlat = [-ca(1,:).*sa(2,:); -sa(1,:).*sa(2,:); ca(2,:)];
    d = reshape([dlon; dlat],[3,2,sz(2)]);
end
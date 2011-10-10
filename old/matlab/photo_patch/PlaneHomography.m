function H = PlaneHomography(p, P, H, z)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% P: Projection matrix
% H: Homgraphy transformation 
% t: translation
% z: center point

p = p/norm(p(1:3)); % normalized plane
v = p(1:3); % normal vector
d = p(4);   % distance from the origin

% spherical angles from pixel coordinates in a orthographic image
a = H*[z;1];        % spherical angles (longitude and latitude)
a = a(1:2)/a(3);    % inhomonizing the coordinates
ca = cos(a);        % cosine of spherical angles 
sa = sin(a);        % sine of spherical angles

% direction vector and its Jacobian
e = [ca(1)*ca(2); sa(1)*ca(2); sa(2)];
de = [-sa(1)*ca(2) -ca(1)*sa(2); ca(1)*ca(2) -sa(1)*sa(2); 0 ca(2)];

Q = d*P(:,1:3)+P(:,4)*v';
u = Q*e;
S = Q*de*H(1:2,1:2);

% homography
H = [S u-S*z];
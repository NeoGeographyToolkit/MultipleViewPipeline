function [J] = PlaneJacobian(p, P, H, c, z)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% P: Projection matrix
% S: Similar transformation
% c: center point
% t: translation

p=p/norm(p(1:3)); % normalized plane
v=p(1:3); % normal vector
d=p(4);   % distance from the origin

e=ang2vec(H,c);
x=d*e/(v'*e);

% direction vector and its Jacobian
[e,de]=ang2vec(H,z);

% correponding point in the orbital image
u=(d*P(:,1:3)+P(:,4)*v')*e;

% intermediate matrices
Q(:,:,1)=x*P(1,1:3)+P(1,4)*eye(3);
Q(:,:,2)=x*P(2,1:3)+P(2,4)*eye(3);
Q(:,:,3)=x*P(3,1:3)+P(3,4)*eye(3);
Q1=u(3)*Q(:,:,1)-u(1)*Q(:,:,3);
Q2=u(3)*Q(:,:,2)-u(2)*Q(:,:,3);
B1=[Q1 [u(3)*P(1,1:3)-u(1)*P(3,1:3)]'];
B2=[Q2 [u(3)*P(2,1:3)-u(2)*P(3,1:3)]'];

% coefficient matrix and its inverse
C=[v'*Q1; v'*Q2]*de;

% Jacobian with respect to the terrain plane
J=-H(1:2,1:2)\C\[e'*B1; e'*B2];

function [e, d]=ang2vec(H,z)
% spherical angles from pixel coordinates in a orthographic image
a = H*[z;1];        % spherical angles (longitude and latitude)
a = a(1:2)/a(3);    % inhomonizing the coordinates
ca = cos(a);        % cosine of spherical angles
sa = sin(a);        % sine of spherical angles

% direction vector and its Jacobian
e = [ca(1)*ca(2); sa(1)*ca(2); sa(2)];
if nargout > 1
    d = [-sa(1)*ca(2) -ca(1)*sa(2); ca(1)*ca(2) -sa(1)*sa(2); 0 ca(2)];
end
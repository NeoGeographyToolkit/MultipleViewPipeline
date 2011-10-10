function [J] = NormalJacobian(p, P, S, t, z)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% P: Projection matrix
% S: Similar transformation 
% t: translation
% z: center point
p = p/norm(p(1:3)); % normalized plane
v = p(1:3); % normal vector
d = p(4);   % distance from the origin

% spherical angles from pixel coordinates in a orthographic image
a = S*z+t;          % spherical angles (longitude and latitude)
ca = cos(a);        % cosine of spherical angles 
sa = sin(a);        % sine of spherical angles

% direction vector and its Jacobian
e = [ca(1)*ca(2); sa(1)*ca(2); sa(2)];
de = [-sa(1)*ca(2) -ca(1)*sa(2); ca(1)*ca(2) -sa(1)*sa(2); 0 ca(2)];
x = d*e/(v'*e);
H = v'*x*P(:,1:3)+P(:,4)*v';
u = H*e;
Q1=x*P(1,1:3)+P(1,4)*eye(3);
Q2=x*P(2,1:3)+P(2,4)*eye(3);
Q3=x*P(3,1:3)+P(3,4)*eye(3);
A1=u(3)*Q1-u(1)*Q3;
A2=u(3)*Q2-u(2)*Q3;
b=[e'*(A1'*v*v'+A1); e'*(A2'*v*v'+A2)];
C=[v'*A1; v'*A2]*de;
dv=-S\C\b;
dr=-S\C\[u(3)*P(1,1:3)-u(1)*P(3,1:3); u(3)*P(2,1:3)-u(2)*P(3,1:3)]*e*v'*e;
J=[dv dr]';
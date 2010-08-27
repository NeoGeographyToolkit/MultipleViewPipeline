function [J] = PseudoJacobian(p, P, S, t, z)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% P: Projection matrix
% S: Similar transformation 
% t: translation
% z: center point

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
C=[v'*Q1; v'*Q2; v'*Q3]*de;
CS=C*S;
dx=-S\(C\[Q1(:,1) Q2(:,1) Q3(:,1)]'*e);
dy=-S\(C\[Q1(:,2) Q2(:,2) Q3(:,2)]'*e);
dz=-S\(C\[Q1(:,3) Q2(:,3) Q3(:,3)]'*e);
dr=-S\(C\P(:,1:3)*e*v'*e);
J=[dx dy dz dr]';
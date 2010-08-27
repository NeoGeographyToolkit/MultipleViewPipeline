function oPatch = mvJacobian(p, iPatch)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% Patch.camera: Projection matrix
% S: Similar transformation
% iPatch.center: center point
% t: translation


p=p/norm(p(1:3)); % normalized plane
v=p(1:3); % normal vector
d=p(4);   % distance from the origin

e=pix2dir(iPatch.georef,iPatch.center);
x=d*e/(v'*e);

a=[iPatch.center(1)-iPatch.width(1)/2:iPatch.center(1)+iPatch.width(1)/2];
b=[iPatch.center(2)-iPatch.width(1)/2:iPatch.center(2)+iPatch.width(1)/2];
[X,Y]=meshgrid(a,b);
z = [X(:) Y(:)]';

sz = size(iPatch.ortho{1});

% direction vector and its Jacobian
oPatch=ort2orb(p,iPatch);

for i=1:numel(iPatch.camera)
    % correponding point in the orbital image
    u=(d*iPatch.camera{i}(:,1:3)+iPatch.camera{i}(:,4)*v')*reshape(e,[sz(1)*sz(2) 3])';
    u=reshape(u',[sz 3]);

    % intermediate matrices
    Q(:,:,1)=x*iPatch.camera{i}(1,1:3)+iPatch.camera{i}(1,4)*eye(3);
    Q(:,:,2)=x*iPatch.camera{i}(2,1:3)+iPatch.camera{i}(2,4)*eye(3);
    Q(:,:,3)=x*iPatch.camera{i}(3,1:3)+iPatch.camera{i}(3,4)*eye(3);
end
Q1=u(3)*Q(:,:,1)-u(1)*Q(:,:,3);
Q2=u(3)*Q(:,:,2)-u(2)*Q(:,:,3);
B1=[Q1 [u(3)*iPatch.camera(1,1:3)-u(1)*iPatch.camera(3,1:3)]'];
B2=[Q2 [u(3)*iPatch.camera(2,1:3)-u(2)*iPatch.camera(3,1:3)]'];

% coefficient matrix and its inverse
C=[v'*Q1; v'*Q2]*de;

% Jacobian with respect to the terrain plane
J=-iPatch.georef(1:2,1:2)\C\[e'*B1; e'*B2];
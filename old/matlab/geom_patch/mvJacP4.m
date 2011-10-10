function [J, Patch] = mvJacP4(d, Patch)
% p: distance from the origin
% Patch.camera: Projection matrix
% S: Similar transformation
% Patch.center: center point
% t: translation

if ~isfield(Patch,'point')
    Patch.point = Patch.center;
end
v=Patch.plane(1:3);
e=pix2dir(Patch.georef,Patch.center);
x=d*e/(v'*e);

[e,de]=pix2dir(Patch.georef,Patch.point);
J = [];
for i=1:numel(Patch.camera)
    % correponding point in the orbital image
    u=(d*Patch.camera{i}(:,1:3)+Patch.camera{i}(:,4)*v')*e;
    
    if nargout > 1 
        Patch.sight{i} = u;
    end
    
    % intermediate matrices
    Q(:,:,1)=x*Patch.camera{i}(1,1:3)+Patch.camera{i}(1,4)*eye(3);
    Q(:,:,2)=x*Patch.camera{i}(2,1:3)+Patch.camera{i}(2,4)*eye(3);
    Q(:,:,3)=x*Patch.camera{i}(3,1:3)+Patch.camera{i}(3,4)*eye(3);

    Q1=u(3)*Q(:,:,1)-u(1)*Q(:,:,3);
    Q2=u(3)*Q(:,:,2)-u(2)*Q(:,:,3);
    B1=[u(3)*Patch.camera{i}(1,1:3)-u(1)*Patch.camera{i}(3,1:3)]';
    B2=[u(3)*Patch.camera{i}(2,1:3)-u(2)*Patch.camera{i}(3,1:3)]';

    % coefficient matrix and its inverse
    C=[v'*Q1; v'*Q2]*de;

    % Jacobian with respect to the terrain plane
    J=[J -Patch.georef(1:2,1:2)\(C\[e'*B1; e'*B2])];
end
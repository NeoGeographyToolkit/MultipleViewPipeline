function J = ortho_proj(I,P,p,H,c)
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% I: Orbital images
% P: Projection matrices
% H: Homgraphy transformation
% c: center point

p = p/norm(p(1:3)); % normalized plane
v = p(1:3); % normal vector
d = p(4);   % distance from the origin

% direction vector and its Jacobian
[e, de] = pix2dir(H,c);

sz=size(P);

for i=1:sz(3)
    Q = d*P(:,1:3,i)+P(:,4,i)*v';
    u = Q*e;
    S = Q*de*H(1:2,1:2);

    % homography
    Hp = [S u-S*z];

    tform = maketform('projective',Hp);
    J = imtransform(I,tform,'bicubic','size',[10 10]);
end
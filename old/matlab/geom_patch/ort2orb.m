function oPatch=ort2orb(p,iPatch)
% compute spherical direction from orthographic image coordinates.
p=p/norm(p(1:3)); % normalized plane
v=p(1:3); % normal vector
d=p(4);   % distance from the origin

H = iPatch.georef/iPatch.georef(3,3);
if H(1,2) == 0 & H(2,1) == 0 & H(3,1) == 0 & H(3,2) == 0
    sz = size(iPatch.ortho{1});
    % correlation and smoothing windows
    gc = diff(normcdf([-iPatch.width(1)/2-0.5:iPatch.width(1)/2+0.5],0,iPatch.width(1)/6));
    gc = gc/sum(gc);
    gs = diff(normcdf([-iPatch.width(2)/2-0.5:iPatch.width(2)/2+0.5],0,iPatch.width(2)/6));
    gs = gs/sum(gs);
    gd = diff(normpdf([-iPatch.width(2)/2-0.5:iPatch.width(2)/2+0.5],0,iPatch.width(2)/6));

    % center point
    e0=pix2dir(iPatch.georef,iPatch.center);
    x0=d*e0/(v'*e0);


    x=[iPatch.center(1)-iPatch.width(1)/2:iPatch.center(1)+iPatch.width(1)/2];
    y=[iPatch.center(2)-iPatch.width(1)/2:iPatch.center(2)+iPatch.width(1)/2];
    nx = length(x);
    ny = length(y);

    ax = H(1,1)*x(:)+H(1,3);
    ay = H(2,2)*y(:)+H(2,3);
    cx = cos(ax);
    cy = cos(ay);
    sx = sin(ax);
    sy = sin(ay);

    e(:,:,1) = cy*cx';
    e(:,:,2) = cy*sx';
    e(:,:,3) = repmat(sy,[1 nx]);

    % Jacobian of longitude and latitude
    dlon(:,:,1) = -e(:,:,2);
    dlon(:,:,2) = e(:,:,1);
    dlon(:,:,3) = zeros(ny,nx);
    dlon = reshape(dlon,[nx*ny 3])';

    dlat(:,:,1) = -sy*cx';
    dlat(:,:,2) = -sy*sx';
    dlat(:,:,3) = repmat(cy,[1 nx]);
    dlat = reshape(dlat,[nx*ny 3])';

    for i=1:numel(iPatch.camera)
        % gradient image
        dx = conv2(gs,gd,iPatch.ortho{i},'same');
        dy = conv2(gd,gs,iPatch.ortho{i},'same');
        % intermediate matrices
        Q=[x0*iPatch.camera{i}(1,1:3)+iPatch.camera{i}(1,4)*eye(3) ...
            x0*iPatch.camera{i}(2,1:3)+iPatch.camera{i}(2,4)*eye(3) ...
            x0*iPatch.camera{i}(3,1:3)+iPatch.camera{i}(3,4)*eye(3)];

        u = [d*iPatch.camera{i}(:,1:3)+iPatch.camera{i}(:,4)*v'; ...
            Q'; iPatch.camera{i}(:,1:3)]*reshape(e,[nx*ny 3])';
        oPatch.U{i} = reshape(u(1:3,:)',[ny nx 3]);

        QA = reshape(v'*Q,[3,3])';
        Qu = QA*dlon;
        c11 = u(3,:).*Qu(1,:)-u(1,:).*Qu(3,:);
        c21 = u(3,:).*Qu(2,:)-u(2,:).*Qu(3,:);
        Qu = QA*dlat;
        c12 = u(3,:).*Qu(1,:)-u(1,:).*Qu(3,:);
        c22 = u(3,:).*Qu(2,:)-u(2,:).*Qu(3,:);

        D = c11.*c22-c21.*c12;
        for k=1:4
            for j=1:2
                str=sprintf('b%d%d = u(3,:).*u(%d,:)-u(%d,:).*u(%d,:);', ...
                    j,k,          3*k+j,      j,   3*k+3);
                eval(str);
            end
            str=sprintf('oPatch.j{%d,1,%d} = gc*reshape(transpose(dx(:)).*(c22.*b1%d-c12.*b2%d)./D,sz)*transpose(gc);', i, k, k, k);
            eval(str);
            str=sprintf('oPatch.j{%d,2,%d} = gc*reshape(transpose(dy(:)).*(c11.*b2%d-c21.*b1%d)./D,sz)*transpose(gc);', i, k, k, k);
            eval(str);
        end

    end
end
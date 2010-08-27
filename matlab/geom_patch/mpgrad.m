function g = mpgrad(p,Patch)
% compute spherical direction from orthographic image coordinates.
% p=p/norm(p(1:3)); % normalized plane
v=p(1:3); % normal vector
d=p(4);   % distance from the origin

H = Patch.georef/Patch.georef(3,3);
if H(1,2) == 0 & H(2,1) == 0 & H(3,1) == 0 & H(3,2) == 0
    g = zeros(4,1);

    sz = size(Patch.ortho{1});
    % correlation and smoothing windows
    gc = diff(normcdf([-Patch.width(1)/2-0.5:Patch.width(1)/2+0.5],0,Patch.width(1)/6));
    gc = gc/sum(gc);
    gs = diff(normcdf([-Patch.width(2)/2-0.5:Patch.width(2)/2+0.5],0,Patch.width(2)/6));
    gs = gs/sum(gs);
    gd = diff(normpdf([-Patch.width(2)/2-0.5:Patch.width(2)/2+0.5],0,Patch.width(2)/6));

    % center point
    e0=pix2dir(Patch.georef,Patch.center);
    x0=d*e0/(v'*e0);

    %
    x=[Patch.center(1)-Patch.width(1)/2:Patch.center(1)+Patch.width(1)/2];
    y=[Patch.center(2)-Patch.width(1)/2:Patch.center(2)+Patch.width(1)/2];
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

    Qidx = [1 4 7 2 5 8 3 6 9];
    for i=1:numel(Patch.camera)
        % gradient image
        dx = conv2(gs,gd,Patch.ortho{i},'same');
        dy = conv2(gd,gs,Patch.ortho{i},'same');
        % intermediate matrices
        Q = [x0*Patch.camera{i}(1,1:3)+Patch.camera{i}(1,4)*eye(3) ...
             x0*Patch.camera{i}(2,1:3)+Patch.camera{i}(2,4)*eye(3) ...
             x0*Patch.camera{i}(3,1:3)+Patch.camera{i}(3,4)*eye(3)];

        u = [d*Patch.camera{i}(:,1:3)+Patch.camera{i}(:,4)*v'; ...
             Q(:,Qidx)'; Patch.camera{i}(:,1:3)]*reshape(e,[nx*ny 3])';

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
            str1=sprintf('g(%d) = g(%d) + gc*reshape((dx(:)''.*(c22.*b1%d', k, k, k);
            str2=sprintf('-c12.*b2%d)+dy(:)''.*(c11.*b2%d-c21.*b1%d))./D,sz)*gc'';', k, k, k);
            eval([str1 str2]);
        end
    end
end
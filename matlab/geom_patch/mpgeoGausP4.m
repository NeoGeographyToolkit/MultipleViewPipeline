function [f,g]=mpgeoGausP4(p,Patch)
% Patch-wise multiple view correlator
% p: plane parmeters p(1:3) normal vector p(4) distance from the origin
% I: Orbital images
% P: Projection matrices
% Patch.georef: Homgraphy transformation
% Patch.center: center point

v = Patch.plane(1:3); % normal vector
d = p;   % distance from the origin

% correlation and smoothing windows
gc = diff(normcdf([-Patch.width(1)/2-0.5:Patch.width(1)/2+0.5],0,Patch.width(1)/6));
gc = gc/sum(gc);
gs = diff(normcdf([-Patch.width(2)/2-0.5:Patch.width(2)/2+0.5],0,Patch.width(2)/6));
gs = gs/sum(gs);

Patch.plane(4) = p+1;
rPatch=mporthoproj(Patch);
Patch.plane(4) = p-1;
lPatch=mporthoproj(Patch);
Patch.plane(4) = p;
Patch=mporthoproj(Patch);
n = numel(Patch.ortho);
for i=1:n
    Is(:,:,i) = conv2(gs,gs,Patch.ortho{i},'same');
    Ik(:,:,i) = Patch.ortho{i};
end
Im = mean(Is,3);
I2 = mean(Is.^2,3);
f = gc*(I2-Im.^2)*gc';

%     Im = conv2(gs,gs,Patch.ortho{1},'same');
%     f = gc*Im.^2*gc';
%     n = numel(Patch.ortho);
%     for i=2:n
%         Is = conv2(gs,gs,Patch.ortho{i},'same');
%         f = f + gc*Is.^2*gc';
%         Im = Im + Is;
%     end
%     Im = Im/n;
%     f = f - n*gc*Im.^2*gc';

% Gradient of the objective function
if nargout > 1
    H = Patch.georef;
    if H(1,2) == 0 & H(2,1) == 0 & H(3,1) == 0 & H(3,2) == 0
        g = zeros(4,1);

        sz = size(Patch.ortho{1});
        % correlation and smoothing windows
        ds = diff(normpdf([-Patch.width(2)/2-0.5:Patch.width(2)/2+0.5],0,Patch.width(2)/6));
%        dc = diff(normpdf([-Patch.width(1)/2-0.5:Patch.width(1)/2+0.5],0,Patch.width(1)/6));

        % center point
        e0=pix2dir(H,Patch.center);
        x0=d*e0/(v'*e0);

        % longitude and latitude grids
        x=[Patch.center(1)-Patch.width(1)/2:Patch.center(1)+Patch.width(1)/2];
        y=[Patch.center(2)-Patch.width(1)/2:Patch.center(2)+Patch.width(1)/2];
        nx = length(x); ny = length(y);

        ax = H(1,1)*x(:)+H(1,3);    ay = H(2,2)*y(:)+H(2,3);
        cx = cos(ax); cy = cos(ay); sx = sin(ax); sy = sin(ay);

        e(:,:,1) = cy*cx';
        e(:,:,2) = cy*sx';
        e(:,:,3) = repmat(sy,[1 nx]);
        e=reshape(e,[nx*ny 3])';

        % Jacobian of e wrt longitude
        dlon= [-e(2,:); e(1,:); zeros(1,ny*nx)];

        % Jacobian of e wrt latitude
        dlat(:,:,1) = -sy*cx';
        dlat(:,:,2) = -sy*sx';
        dlat(:,:,3) = repmat(cy,[1 nx]);
        dlat = reshape(dlat,[nx*ny 3])';

        Qidx = [1 4 7 2 5 8 3 6 9];
        Ir = mean(Ik,3);
        for i=1:n
            Ie=Is(:,:,i)-Im;
            % gradient image
            Id=Patch.ortho{i}-Ir;
            Ix = conv2(gs,ds,Id,'same');
            Iy = conv2(ds,gs,Id,'same');
            
            % intermediate matrices
            Q = [x0*Patch.camera{i}(1,1:3)+Patch.camera{i}(1,4)*eye(3) ...
                x0*Patch.camera{i}(2,1:3)+Patch.camera{i}(2,4)*eye(3) ...
                x0*Patch.camera{i}(3,1:3)+Patch.camera{i}(3,4)*eye(3)];

            u = [d*Patch.camera{i}(:,1:3)+Patch.camera{i}(:,4)*v'; ...
                Q(:,Qidx)'; Patch.camera{i}(:,1:3)]*e;

            QA = reshape(v'*Q,[3,3])';
            Qu = QA*dlon;
            c11 = u(3,:).*Qu(1,:)-u(1,:).*Qu(3,:);
            c21 = u(3,:).*Qu(2,:)-u(2,:).*Qu(3,:);
            Qu = QA*dlat;
            c12 = u(3,:).*Qu(1,:)-u(1,:).*Qu(3,:);
            c22 = u(3,:).*Qu(2,:)-u(2,:).*Qu(3,:);

            D = c11.*c22-c21.*c12;
            for k=4:4
                for j=1:2
                    str=sprintf('b%d%d = u(3,:).*u(%d,:)-u(%d,:).*u(%d,:);', ...
                        j,k,          3*k+j,      j,   3*k+3);
                    eval(str);
                end
                str=sprintf('zx = (c12.*b2%d-c22.*b1%d)/H(1,1);',k,k);
                eval(str);
                str=sprintf('zy = (c21.*b1%d-c11.*b2%d)/H(2,2);',k,k);
                eval(str);
                g(k) = g(k) + gc*reshape((Ix(:).*zx'-Iy(:).*zy').*Ie(:)./D',sz)*gc';
            end
        end
    end
    g = 2*g(4)/n;
end
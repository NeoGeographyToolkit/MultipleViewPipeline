classdef SingleView < handle
    % data: img, cam, back-projected tile
    % patch: e,q,r,s,t,u -> p
    % status: visible, half-visible, invisible
    properties (Constant)
        ratioSmoth = 5;
        eps_sqrt = sqrt(eps);
    end % properties (Constant)
    
    properties (Hidden)
        img
        cam
        wat
        
        a = 1;
        b = 0;
        c = [20 20]';   % clipping window size
        t = [0 0]';     % translation
        e = [0 0 0]';   % elevation post
        q = [0 0 0 1]'; % quaternion for rotation matrix
        r = RasterView.radiusMoon;    % radius
        r0 = RasterView.radiusMoon;   % reference radius
        dr = 0;         % offset of radius
        s = 1;          % smoothing sigma
        u = 31;
    end % properties (Hidden)
    
    properties (Hidden)     % tiled projection
        cI  % cell images
        cW  % cell weights
        tC = [1 1]';
        tW  % tiled weight
        tM = true(3);   % tiled map
        tX, tY;
        tw, tD;
        
        d = 40;             % radius of tile
    end % properties (Hidden)
    
    properties (SetObservable, SetAccess = public)
        h = [0 0 0]';       % parameter of affine transform
        W                   % inlier membership
    end
    
    properties (SetObservable, SetAccess = private)
        w = [20 20 3];      % window size for correlation and smoothing
    end
    
    properties (SetAccess = private)
        camDet
        camCen
        
        ce = [0 0 0]';      % cross of camCen and e
        
        p = [0 0 0]';       % origin of patch % 3d point
        v = [0 0 0]';       % viewing vector
        H = eye(3);         % compute homography
        Hi = eye(3);        % inverse homography
        Hr = eye(3);        % homography of image formation
        Hs = eye(3);        % inverse homography due to sloppy refinement
        R = eye(3);         % rotation matrix of q
        tF                  % transform
        
        xb = 16*[-1 1]; % xdata of backward projection
        yb = 16*[-1 1]; % ydata of backward projection
        is = [1:33];    % effective  row   indices after smoothing
        js = [1:33];    % effective column indices after smoothing
        X               % X
        Y
        
        s0, s1, s2
        s0x, s1x, s2x
        s0y, s1y, s2y
        r3 = 1, x3 = 1, y3 = 1;
        uvz = 1;    % u*v*ez
        
        Ik, Wb  % (backward) projected image and weight
        Is, Ws  % smoothed weighted image and weight
        Ix, Wx  % gradient weighted image and weight
        Iy, Wy  % gradient weighted image and weight
        Wk      % =W*Wb
        
        tIk, tWb  % (backward) projected image and weight
        tIs, tWs  % smoothed weighted image and weight
        tIx, tWx  % gradient weighted image and weight
        tIy, tWy  % gradient weighted image and weight
        tWk      % =W*Wb
        tI, tJ;
        tCen
    end
    
    methods
        function sv = SingleView(img,cam)
            % Constructor assigns property values
            if nargin > 0,
                sv.img = img;
            end
            if nargin > 1,
                sv.cam = cam;
                sv.camDet = det(sv.cam(:,1:3));
                sv.camCen = -sv.cam(:,1:3)\sv.cam(:,4);
            end
            addlistener(sv,'h','PostSet',@sv.PropEvents);
            addlistener(sv,'W','PostSet',@sv.PropEvents);
            sv.tw = 2*sv.d+1;
            tx = -sv.tw:sv.tw:sv.tw;
            [sv.tX,sv.tY] = meshgrid(tx,tx);    % center of tile
            sv.tD = sv.d*[-1 1];
            sv.tCen = sv.tw+sv.d+1+round(sv.t);
        end
        
        function PropEvents(sv,src,evt)
            switch src.Name
                case 'c'
                    c = evt.AffectedObject.c;
                    if ~isequal(sv.c,c),
                        sv.c = c;
                        sv.xb = sv.c(1)*[-1 1]; % xdata
                        sv.yb = sv.c(2)*[-1 1]; % ydata
                        x=sv.xb(1):sv.xb(2); y=sv.yb(1):sv.yb(2);
                        [sv.X, sv.Y]=meshgrid(x,y);
                        sdx = sv.c-sv.w(1:2)+1; edx = 2*sv.w(1:2)+sdx;
                        sv.is = sdx(2):edx(2); sv.js = sdx(1):edx(1);
                        sv.proj;
                        sv.W = ones(size(sv.X));
                    end
                case 'e'    % elevation post from PatchViews
                    e = evt.AffectedObject.e;
                    if ~isequal(sv.e,e),
                        sv.e = e;
                        sv.ce = cross(sv.camCen,sv.e);
                        % initialize q
                        ex = [-sv.e(2) sv.e(1) 0]'; ex = ex/norm(ex);
                        ey = -cross(sv.e,ex); ey = ey/norm(ey);
                        sv.R = [ex ey -sv.e];
                        sv.q = dcm2q(sv.R)';
                        sv.rotate;
                        sv.elevate;
                        sv.proj;
                    end
                case 'h'
                    c = cos(sv.h(3)); s = sin(sv.h(3)); R = [c s; -s c];
                    sv.Hs = [R -sv.h(1:2); 0 0 1];
                    sv.proj;
                case 'q'    % quaternion from PatchViews
                    q = evt.AffectedObject.q;
                    if ~isequal(sv.q,q),
                        sv.q = q;
                        sv.R = q2dcm(sv.q);
                        sv.rotate
                        sv.proj;
                        sv.t = [0 0]';
                    end
                case 'r'    % elevation from PatchViews
                    r = evt.AffectedObject.r;
                    if ~isequal(sv.r,r),
                        sv.r = r;
                        sv.elevate;
                        sv.proj;
                        sv.t = sv.dr*[sv.R(:,2) -sv.R(:,1)]'*sv.ce/sv.uvz;
                    end
                case 's'    % smoothing scale from PatchViews
                    s = evt.AffectedObject.s;
                    if ~isequal(sv.s,s),
                        sv.s = s;
                        sv.w(3)=ceil(sv.s*SingleView.ratioSmoth);
                        [sv.s0 sv.s1 sv.s2]=fltGaussian(sv.w(3),sv.s);
                        sv.smooth;
                        
                        t = sv.t-round(sv.t);
                        [sv.s0x sv.s1x sv.s2x]=fltGaussian(sv.w(3),sv.s,t(1));
                        [sv.s0y sv.s1y sv.s2y]=fltGaussian(sv.w(3),sv.s,t(2));
                    end
                case 'w'
                    w = evt.AffectedObject.w;
                    if ~isequal(sv.w,w),
                        sv.w = w;
                        sdx = sv.c-sv.w(1:2)+1; edx = 2*sv.w(1:2)+sdx;
                        sv.is = sdx(2):edx(2); sv.js = sdx(1):edx(1);

%                         [sv.X, sv.Y]=meshgrid(x,y);
%                         sv.tI = sv.tCen(2)+y;
%                         sv.tJ = sv.tCen(1)+x;
                        [sv.s0 sv.s1 sv.s2]=fltGaussian(sv.w(3),sv.s);
%                         t = sv.t-round(sv.t);
%                         [sv.s0x sv.s1x sv.s2x]=fltGaussian(sv.w(3),sv.s,t(1));
%                         [sv.s0y sv.s1y sv.s2y]=fltGaussian(sv.w(3),sv.s,t(2));
                    end
                case 'W'
                    sv.smooth;
            end
        end
        
        function elevate(sv)
            sv.p = sv.r*sv.e;
            sv.v = sv.p-sv.camCen;
            sv.Hr(:,3) = sv.cam(:,1:3)*sv.p+sv.cam(:,4);
            sv.r3 = sv.cam(3,1:3)*sv.v;
            sv.uvz = sv.u*sv.v'*sv.R(:,3);
        end
        
        function rotate(sv)
            sv.Hr(:,1:2) = sv.u*sv.cam(:,1:3)*sv.R(:,1:2);
            sv.x3 = sv.Hr(3,1); sv.y3 = sv.Hr(3,2);
            sv.uvz = sv.u*sv.v'*sv.R(:,3);
        end
        
        function c = grad_se(sv,Wc,Ms,Ga,Gb)
            Gw = Wc.*(Ga-Ms); Mw = Gw.*(Ga+Ms);
            Gw = conv2(sv.s0,sv.s0,Gw,'same');
            Mw = conv2(sv.s0,sv.s0,Mw,'same');
            c = sv.Wb.*(Gb.*Gw-Mw/2);
        end
        
        function c = grad_ss(sv,Wc,Gb,Ms,Mx,My,Wt,Wx,Wy)
            Wcs = Wc./Wt; Wcs(isnan(Wcs)) = 0;
            gx = Mx.*Wcs; gy = My.*Wcs;
            s21 = conv2(sv.s0,sv.s1,gx,'same')+conv2(sv.s1,sv.s0,gy,'same');
            S22 = (gx.*Wx+gy.*Wy)./Wt; S22(isnan(S22)) = 0;
            s22 = conv2(sv.s0,sv.s0,S22,'same');

            s11 = conv2(sv.s0,sv.s1,gx.*Ms,'same')+conv2(sv.s1,sv.s0,gy.*Ms,'same');
            s12 = conv2(sv.s0,sv.s0,S22.*Ms,'same');

            S3 = (Mx.*gx+My.*gy)./Wt; S3(isnan(S3)) = 0;
            s3 = conv2(sv.s0,sv.s0,S3,'same')/2;
            c = sv.Wb.*(s11+s12-Gb.*(s21+s22)-s3);
        end
        
        function c = grad_ms(sv,Wc,Wt,Gb,Ms)
            Wcs = Wc./Wt; Wcs(isnan(Wcs)) = 0;
            Gc = conv2(sv.s0,sv.s0,Wcs,'same');
            Mc = conv2(sv.s0,sv.s0,Wcs.*Ms,'same');
            c = sv.Wb.*(Gb.*Gc-Mc);
        end
        
        function c = grad_mx(sv,Wc,Wt,Wx,Gb,Ms)
            gx = sv.grad_gx(Wc,Gb);

            % ms*wx
            Wcs = Wx.*Wc./Wt; Wcs(isnan(Wcs)) = 0;
            Gc = conv2(sv.s0,sv.s0,Wcs,'same');
            Mc = conv2(sv.s0,sv.s0,Wcs.*Ms,'same');
            ms = sv.Wb.*(Gb.*Gc-Mc);
            
            % ms*wx
            wx = -sv.Wb.*conv2(sv.s0,sv.s1,Wc.*Ms,'same');
            c = gx - ms - wx;
        end
        
        function c = grad_my(sv,Wc,Wt,Wy,Gb,Ms)
            gy = sv.grad_gy(Wc,Gb);

            % ms*wx
            Wcs = Wy.*Wc./Wt; Wcs(isnan(Wcs)) = 0;
            Gc = conv2(sv.s0,sv.s0,Wcs,'same');
            Mc = conv2(sv.s0,sv.s0,Wcs.*Ms,'same');
            ms = sv.Wb.*(Gb.*Gc-Mc);
            
            % ms*wx
            wy = -sv.Wb.*conv2(sv.s1,sv.s0,Wc.*Ms,'same');
            c = gy - ms - wy;
        end
        
        function c = grad_gs(sv,Wc,Gb)
            c = sv.Wb.*Gb.*conv2(sv.s0,sv.s0,Wc,'same');
        end
        
        function c = grad_gx(sv,Wc,Gb)
            c = -sv.Wb.*Gb.*conv2(sv.s0,sv.s1,Wc,'same');
        end
        
        function c = grad_gy(sv,Wc,Gb)
            c = -sv.Wb.*Gb.*conv2(sv.s1,sv.s0,Wc,'same');
        end
        
        function c = grad_ws(sv,Wc)
            c = sv.Wb.*conv2(sv.s0,sv.s0,Wc,'same');
        end
        
        function c = grad_wx(sv,Wc)
            c = -sv.Wb.*conv2(sv.s0,sv.s1,Wc,'same');
        end
        
        function c = grad_wy(sv,Wc)
            c = -sv.Wb.*conv2(sv.s1,sv.s0,Wc,'same');
        end
        
        function c = grad_nt(sv,c)
            c = sv.Wb.*conv2(sv.s0,sv.s0,c,'same');
        end
        
        function c = grad_ns(sv,Wc,Wr,Wt)
            % Wc: correlation winodw
            % Wr: signal dof
            % Wt: total dof
            c = Wc.*(2*sv.Ws-Wr)./Wt; c(isnan(c)) = 0;
            c = sv.Wb.*conv2(sv.s0,sv.s0,c,'same');
        end
        
        function set.q(obj,q)
            % q property set function
            if  length(q) == 4
                if q(:) == zeros(4,1),
                    error('q must be a non-zero vector')
                else
                    obj.q = q/norm(q);
                end
            else
                error('q must be a 4-dimensional non-zero vector')
            end
        end % set.q
        
        function set.r(obj,r)
            % r property set function
            if isscalar(r)
                obj.r = r;
            else
                error('r must be a scalar value')
            end
        end % set.q
        
        function set.a(obj,a)
            % a property set function
            if isscalar(a)
                obj.a = a;
            else
                error('a must be a scalar value')
            end
        end % set.a
        
        function set.b(obj,b)
            % b property set function
            if isscalar(b)
                obj.b = b;
            else
                error('b must be a scalar value')
            end
        end % set.b
        
        function set.h(obj,h)
            % h property set function
            if length(h) == 3
                obj.h = h;
            else
                error('h must be a 3-vector')
            end
        end % set.h
        
        function set.s(obj,s)
            % s property set function
            if isscalar(s) && s > 0
                obj.s = s;
            else
                error('s must be a positive scalar')
            end
        end % set.s
        
        function set.W(obj,W)
            % W property set function
            if any(W(:) > 1 | W(:) < 0)
                fprintf('All elements of W are set be probabilities\n')
                W(W>1)=1; W(W<0)=0;
            end
            obj.W = W;
        end % set.W
        
        function set.img(obj,img)
            % Camera property set function
            obj.img = img;
        end % set.Camera
        
        function set.cam(obj,cam)
            % Camera property set function
            obj.cam = cam;
        end % set.Camera
        
        function set.camDet(obj,camDet)
            % Camera property set function
            obj.camDet = camDet;
        end % set.camDet
        
        function camDet = get.camDet(obj)
            % Camera property get function
            camDet = obj.camDet;
        end % get.camDet
        
        function set.camCen(obj,camCen)
            % Camera property set function
            obj.camCen = camCen;
        end % set.camCen
        
        function camCen = get.camCen(obj)
            % Camera property get function
            camCen = obj.camCen;
        end % get.camCen
        
        function h = disp(sv,h)
            if nargin < 2, h = figure; else figure(h), end
            imshow(sv.img)
        end
        
        function smooth(sv)
            sv.Wk = sv.W.*sv.Wb;
            [sv.Ws,sv.Wx,sv.Wy] = conv3(sv.Wk,sv.s0,sv.s1);
            [sv.Is,sv.Ix,sv.Iy] = conv3(sv.Wk.*sv.Ik,sv.s0,sv.s1);
        end
        
        function [Is,Ws,Ix,Wx,Iy,Wy,Ik,Wb] = crop(sv)
            if nargout > 0, Is = sv.Is(sv.is,sv.js); end
            if nargout > 1, Ws = sv.Ws(sv.is,sv.js); end
            if nargout > 2, Ix = sv.Ix(sv.is,sv.js); end
            if nargout > 3, Wx = sv.Wx(sv.is,sv.js); end
            if nargout > 4, Iy = sv.Iy(sv.is,sv.js); end
            if nargout > 5, Wy = sv.Wy(sv.is,sv.js); end
            if nargout > 6, Ik = sv.Ik(sv.is,sv.js); end
            if nargout > 7, Wb = sv.Wb(sv.is,sv.js); end
        end
        
        function [Is,Ws,Ix,Wx,Iy,Wy,Ik,Wb] = tCrop(sv)
            sv.tProj([1 1 1 2 2 2 3 3 3],[1 2 3 1 2 3 1 2 3]);
            sv.tConv;
            if nargout > 0, Is = sv.tIs(sv.tI,sv.tJ); end
            if nargout > 1, Ws = sv.tWs(sv.tI,sv.tJ); end
            if nargout > 2, Ix = sv.tIx(sv.tI,sv.tJ); end
            if nargout > 3, Wx = sv.tWx(sv.tI,sv.tJ); end
            if nargout > 4, Iy = sv.tIy(sv.tI,sv.tJ); end
            if nargout > 5, Wy = sv.tWy(sv.tI,sv.tJ); end
            if nargout > 6, Ik = sv.tIk(sv.tI,sv.tJ); end
            if nargout > 7, Wb = sv.tWb(sv.tI,sv.tJ); end
        end
    end % methods
    
    methods (Access = private)
        function proj(sv)
            sv.tF = maketform('projective',(sv.Hs/sv.Hr)');
            % backward projection of image and weight
            sv.Ik = imtransform(sv.img,sv.tF,'bicubic','xdata',sv.xb,'ydata',sv.yb);
            %            sv.Wb = imtransform(sv.wat,tform,'bicubic','xdata',sv.xb,'ydata',sv.yb);
            
            % determinant of backward Jacobian
            d = sv.x3*sv.X+sv.y3*sv.Y+sv.r3;
            %            n = -sv.u^2*sv.camDet*sv.vz;
            n = abs(sv.u*sv.camDet*sv.uvz);
            sv.Wb = d.^3/n;
            
            %             D = abs(sv.H(3,3)^3/det(sv.H)); % determinant of jacobian
            %             if abs(sv.Wb(sv.yb(2)+1,sv.xb(2)+1)-D) > SingleView.eps_sqrt
            %                 figure, imagesc(sv.Wb), colorbar
            %                 error('Jacobian should be similar to numerical estimate!!!')
            %             end
            if any(sv.Wb(:) < 0),
                sv.R, sv.q, sv.r
                figure, imagesc(sv.Wb), colorbar
                fprintf('Weight should be nonnegative!!!')
            end
        end
        
        function tProj(sv,i,j)
            % backward projection of image and weight
            m = length(i);
            %            n = -sv.u^2*sv.camDet*sv.vz;
            n = abs(sv.u*sv.camDet*sv.uvz);
            for k = 1:m
                xb = sv.tX(i(k),j(k))+sv.tD; yb = sv.tY(i(k),j(k))+sv.tD;
                sv.cI{i(k),j(k)} = imtransform(sv.img,sv.tF,'bicubic','xdata',xb,'ydata',yb);

                % determinant of backward Jacobian
                d = sv.x3*(sv.X+sv.tX(i(k),j(k)))+sv.y3*(sv.Y+sv.tY(i(k),j(k)))+sv.r3;
                sv.cW{i(k),j(k)} = d.^3/n;
            end
            sv.tIk = cell2mat(sv.cI);
            sv.tWb = cell2mat(sv.cW);
            
            %             D = abs(sv.H(3,3)^3/det(sv.H)); % determinant of jacobian
            %             if abs(sv.Wb(sv.yb(2)+1,sv.xb(2)+1)-D) > SingleView.eps_sqrt
            %                 figure, imagesc(sv.Wb), colorbar
            %                 error('Jacobian should be similar to numerical estimate!!!')
            %             end
            if any(sv.tW(:) < 0),
                sv.R, sv.q, sv.r
                figure, imagesc(sv.tW), colorbar
                error('Weight should be nonnegative!!!')
            end
        end
        
        function tConv(sv,i,j)
            sv.tWk = sv.tWb;
            sv.tWs=conv2(sv.s0y,sv.s0x,sv.tWk,'same');
            sv.tWx=conv2(sv.s0y,sv.s1x,sv.tWk,'same');
            sv.tWy=conv2(sv.s1y,sv.s0x,sv.tWk,'same');
            tIW = sv.tWk.*sv.tIk;
            sv.tIs=conv2(sv.s0y,sv.s0x,tIW,'same');
            sv.tIx=conv2(sv.s0y,sv.s1x,tIW,'same');
            sv.tIy=conv2(sv.s1y,sv.s0x,tIW,'same');
        end
    end
    
    methods (Static)
    end % methods (Access = private)
end % classdef

function [Is,Ix,Iy] = conv3(I,s0,s1)
Is=conv2(s0,s0,I,'same');
Ix=conv2(s0,s1,I,'same');
Iy=conv2(s1,s0,I,'same');
end

function [f0 f1 f2]=fncGaussian(r,s,t)
% r: radius, s: sigma
x=[-r-0.5:r+0.5]'+t; fx = normpdf(x,0,s);
f0 = diff(normcdf(x,0,s));
f1 = diff(fx);
f2 = diff(-x.*fx)/s^2;
end

function [f0 f1 f2]=fltGaussian(r,s,c)
% r: radius, s: sigma, c: offset
if nargin < 3, c = 0; end
[f0 f1 f2]=fncGaussian(r,s,c);
n0 = sum(f0); f0 = f0/n0; f1 = f1/n0; f2 = f2/n0;
end
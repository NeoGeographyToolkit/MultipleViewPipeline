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
        e = [0 0 0]';   % elevation post
        q = [0 0 0 1]'; % quaternion for rotation matrix
        r = 1737400;    % radius of the moon
        s = 1;          % smoothing sigma
        u = 31;
    end % properties (Hidden)
    
    properties (SetObservable, SetAccess = public)
        h = [0 0 0]';       % parameter of affine transform
        W                   % inlier membership
    end
    
    properties (SetObservable, SetAccess = private)
        w = [20 20 3];      % window size for correlation and smoothing
        p = [0 0 0]';       % origin of patch % 3d point
        v = [0 0 0]';       % viewing vector
        R = eye(3);         % rotation matrix of q
        S = [0 0 0]';       % Projected Patch Center (sv.p)
    end
    
    properties (SetAccess = private)
        camDet
        camCen
        
        H = eye(3);         % compute homography
        Hr = eye(3);        % homography of image formation
        Hs = eye(3);        % homography due to sloppy refinement
        
        xb = 16*[-1 1]; % xdata of backward projection
        yb = 16*[-1 1]; % ydata of backward projection
        is = [1:33];    % effective  row   indices after smoothing
        js = [1:33];    % effective column indices after smoothing
        X               % X
        Y
        
        imgCen
        tilImg
        tilWat
        tilMap
        
        s0, s1, s2
        r3, x3, y3
        
        Ik, Wb  % (backward) projected image and weight
        Is, Ws  % smoothed weighted image and weight
        Ix, Wx  % gradient weighted image and weight
        Iy, Wy  % gradient weighted image and weight
        Wk      % =W*Wb
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
            addlistener(sv,'p','PostSet',@sv.PropEvents);
            addlistener(sv,'v','PostSet',@sv.PropEvents);
            addlistener(sv,'R','PostSet',@sv.PropEvents);
            addlistener(sv,'S','PostSet',@sv.PropEvents);
            addlistener(sv,'W','PostSet',@sv.PropEvents);
        end
        
        function PropEvents(sv,src,evt)
            switch src.Name
                case 'e'    % elevation post from PatchViews
                    e = evt.AffectedObject.e;
                    if ~isequal(sv.e,e),
                        sv.e = e;
                        sv.p = sv.r*sv.e;
                    end
                case 'h'
                    c = cos(sv.h(3)); s = sin(sv.h(3)); R = [c -s; s c];
                    sv.Hs = [R sv.h(1:2); 0 0 1];
                    sv.H = sv.Hr*sv.Hs;
                case 'p'
                    sv.v = sv.p-sv.camCen;
                    sv.S = sv.cam(:,1:3)*sv.p+sv.cam(:,4);
                case 'q'    % quaternion from PatchViews
                    q = evt.AffectedObject.q;
                    if ~isequal(sv.q,q),
                        sv.q = q;
                        sv.R = q2dcm(sv.q);
                    end
                case 'r'    % elevation from PatchViews
                    r = evt.AffectedObject.r;
                    if ~isequal(sv.r,r),
                        sv.r = r;
                        sv.p = sv.r*sv.e;
                    end
                case 's'    % smoothing scale from PatchViews
                    s = evt.AffectedObject.s;
                    if ~isequal(sv.s,s),
                        sv.s = s;
                        sv.w(3)=ceil(sv.s*SingleView.ratioSmoth);
                        [sv.s0 sv.s1 sv.s2]=fltGaussian(sv.w(3),sv.s);
                    end
                case 'v'
                    sv.r3 = sv.cam(3,1:3)*sv.v;
                case 'w'
                    w = evt.AffectedObject.w;
                    if ~isequal(sv.w,w),
                        sv.w = w;
                        %                         sv.xb = (sv.w(1)+sv.w(3))*[-1 1]; % xdata
                        %                         sv.yb = (sv.w(2)+sv.w(3))*[-1 1]; % ydata
                        %                         x=sv.xb(1):sv.xb(2); y=sv.yb(1):sv.yb(2);
                        %                         [sv.X, sv.Y]=meshgrid(x,y);
                        %                         [sv.s0 sv.s1 sv.s2]=SingleView.fltGaussian(sv.w(3),sv.s);
                        %                         sv.is = sv.w(3)+1:sv.yb(2)-sv.yb(1)+1-sv.w(3);
                        %                         sv.js = sv.w(3)+1:sv.xb(2)-sv.xb(1)+1-sv.w(3);
                        sv.xb = sv.w(1)*[-1 1]; % xdata
                        sv.yb = sv.w(2)*[-1 1]; % ydata
                        x=sv.xb(1):sv.xb(2); y=sv.yb(1):sv.yb(2);
                        [sv.X, sv.Y]=meshgrid(x,y);
                        [sv.s0 sv.s1 sv.s2]=fltGaussian(sv.w(3),sv.s);
                        sv.Ik = ones(size(sv.X));
                        sv.Wb = sv.Ik;
                    end
                case 'S'
                    sv.Hr = [sv.cam(:,1:3)*sv.R(:,1:2)*sv.u sv.S];
                    sv.H = sv.Hr*sv.Hs;
                case 'R'
                    sv.x3 = sv.u*sv.cam(3,1:3)*sv.R(:,1);
                    sv.y3 = sv.u*sv.cam(3,1:3)*sv.R(:,2);
                    sv.Hr = [sv.u*sv.cam(:,1:3)*sv.R(:,1:2) sv.S];
                    sv.H = sv.Hr*sv.Hs;
                case 'W'
                    sv.Wk = sv.W.*sv.Wb;
                    [sv.Ws,sv.Wx,sv.Wy] = smooth(sv.Wk,sv.s0,sv.s1);
                    [sv.Is,sv.Ix,sv.Iy] = smooth(sv.Wk.*sv.Ik,sv.s0,sv.s1);
            end
        end
        
        function c = grad_se(sv,Wc,Ms,Ga,Gb)
            Gw = Wc.*(Ga-Ms); Mw = Gw.*(Ga+Ms);
            Gw = conv2(sv.s0,sv.s0,Gw,'same');
            Mw = conv2(sv.s0,sv.s0,Mw,'same');
            c = sv.Wb.*(2*Gb.*Gw-Mw);
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
        
        function [Is,Ws,Ix,Wx,Iy,Wy,Ik,Wb] = crop(sv)
            sv.proj;
            if nargout > 0, Is = sv.Is; end
            if nargout > 1, Ws = sv.Ws; end
            if nargout > 2, Ix = sv.Ix; end
            if nargout > 3, Wx = sv.Wx; end
            if nargout > 4, Iy = sv.Iy; end
            if nargout > 5, Wy = sv.Wy; end
            if nargout > 6, Ik = sv.Ik; end
            if nargout > 7, Wb = sv.Wb; end
        end
    end % methods
    
    methods (Access = private)
        function [Is,Ws,Ix,Wx,Iy,Wy] = proj(sv)
            % backward projection of image and weight
            tform = maketform('projective',inv(sv.H)');
            sv.Ik = imtransform(sv.img,tform,'bicubic','xdata',sv.xb,'ydata',sv.yb);
            %            sv.Wb = imtransform(sv.wat,tform,'bicubic','xdata',sv.xb,'ydata',sv.yb);
            
            % determinant of backward Jacobian
            d = sv.x3*sv.X+sv.y3*sv.Y+sv.r3;
            %            n = -sv.u^2*sv.camDet*sv.v'*sv.R(:,3);
            n = abs(sv.u^2*sv.camDet*sv.v'*sv.R(:,3));
            sv.Wb = d.^3/n;
            
            %             D = abs(sv.H(3,3)^3/det(sv.H)); % determinant of jacobian
            %             if abs(sv.Wb(sv.yb(2)+1,sv.xb(2)+1)-D) > SingleView.eps_sqrt
            %                 figure, imagesc(sv.Wb), colorbar
            %                 error('Jacobian should be similar to numerical estimate!!!')
            %             end
            if any(sv.Wb(:) < 0),
                sv.R, sv.q, sv.r
                figure, imagesc(sv.Wb), colorbar
                error('Weight should be nonnegative!!!')
            end
            
            sv.Wk = sv.W.*sv.Wb;
            [sv.Ws,sv.Wx,sv.Wy] = smooth(sv.Wk,sv.s0,sv.s1);
            [sv.Is,sv.Ix,sv.Iy] = smooth(sv.Wk.*sv.Ik,sv.s0,sv.s1);
            
            if nargout > 0, Is = sv.Is; end
            if nargout > 1, Ws = sv.Ws; end
            if nargout > 2, Ix = sv.Ix; end
            if nargout > 3, Wx = sv.Wx; end
            if nargout > 4, Iy = sv.Iy; end
            if nargout > 5, Wy = sv.Wy; end
        end
    end
    
    methods (Static)
    end % methods (Access = private)
end % classdef

function [Is,Ix,Iy] = smooth(I,s0,s1)
Is=conv2(s0,s0,I,'same');
Ix=conv2(s0,s1,I,'same');
Iy=conv2(s1,s0,I,'same');
end

function [f0 f1 f2]=fncGaussian(r,s)
% r: radius, s: sigma
x=[-r-0.5:r+0.5]'; fx = normpdf(x,0,s);
f0 = diff(normcdf(x,0,s));
f1 = diff(fx);
f2 = diff(-x.*fx)/s^2;
end

function [f0 f1 f2]=fltGaussian(r,s)
% r: radius, s: sigma
[f0 f1 f2]=fncGaussian(r,s);
n0 = sum(f0); f0 = f0/n0; f1 = f1/n0; f2 = f2/n0;
end
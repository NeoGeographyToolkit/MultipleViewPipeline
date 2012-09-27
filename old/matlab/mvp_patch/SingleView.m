classdef SingleView < handle
    % data: img, cmr, back-projected tile
    % patch: e,q,r,s,t,d -> p
    % status: visible, half-visible, invisible
    properties (Constant)
        ratioSmoth = 5;
        eps_sqrt = sqrt(eps);
    end % properties (Constant)
    
    properties (Hidden)
        img
        cmr
        wat
        
        e = [0 0 1]';   % elevation post
        q = [0 0 0 1]'; % quaternion for rotation matrix
        r = RasterView.radiusMoon;    % radius
        s = 1;          % smoothing sigma
        d = 31;
        W                   % inlier membership
    end % properties (Hidden)
    
    properties (SetObservable, SetAccess = public)
        h = [0 0 0]';       % parameter of affine transform
    end
    
    properties (SetAccess = private)
        dd = 1;
        cmrCen = [0 0 0]';
        
        u = [0 0 0]';      % unit vector of p
        v = [0 0 0]';       % viewing vector
        w = [20 20 3];      % window size for correlation and smoothing
        H = eye(3);         % compute homography
        Hr = eye(3);        % homography of image formation
        Hs = eye(3);        % inverse homography due to sloppy refinement
        R = eye(3);         % rotation matrix of q
        tF                  % transform
        
        xy = [16 16]'*[-1 1]; % xdata of backward projection
        X               % X
        Y
        
        s0, s1, s2
        re = [0 0 0]';
        ecd = [0 0 0]';     % cross of e and cmrCen and e scaled by d
        cr = 0;
        ez = 1;    % e*ez
        vz = 1;    % v*ez
        dvz = 1;    % d*v*ez
        
        Fk,  Gk,  Nk,       % (backward) projected image and weight
        Fa,  Ga,  Na,  Ja,  Wa % smoothed weighted image and weight
        Fx,  Gx,  Nx,  Jx,  Wx % gradient weighted image and weight
        Fy,  Gy,  Ny,  Jy,  Wy % gradient weighted image and weight
        Fr,  Gr,  Nr,  Jr,  Wr
        Frx, Grx, Nrx, Jrx, Wrx
        Fry, Gry, Nry, Jry, Wry  % gradient w.r.t r
    end
    
    methods
        function sv = SingleView(img,cmr)
            % Constructor assigns property values
            if nargin > 0, sv.img = img; end
            if nargin > 1,
                sv.cmr = cmr;
                sv.dd = sv.d^2*det(sv.cmr(:,1:3));
                sv.cmrCen = -sv.cmr(:,1:3)\sv.cmr(:,4);
            end
            addlistener(sv,'h','PostSet',@sv.PropEvents);
        end
        
        function PropEvents(sv,src,evt)
            switch src.Name
                case 'e'    % elevation post from PatchViews
                    e = evt.AffectedObject.e;
                    if ~isequal(sv.e,e),
                        sv.e = e;
                        sv.post;
                        sv.rotate;
                        sv.elevate;
                    end
                case 'h'
                    c = cos(sv.h(3)); s = sin(sv.h(3)); R = [c s; -s c];
                    sv.Hs = [R -sv.h(1:2); 0 0 1];
                case 'q'    % quaternion from PatchViews
                    q = evt.AffectedObject.q;
                    if ~isequal(sv.q,q),
                        sv.q = q;
                        sv.rotate;
                    end
                case 'r'    % elevation from PatchViews
                    r = evt.AffectedObject.r;
                    if ~isequal(sv.r,r),
                        sv.r = r;
                        sv.elevate;
                    end
                case 's'    % smoothing scale from PatchViews
                    s = evt.AffectedObject.s;
                    if ~isequal(sv.s,s),
                        sv.s = s;
                        sv.w(3)=ceil(sv.s*SingleView.ratioSmoth);
                        [sv.s0 sv.s1 sv.s2]=fltGaussian(sv.w(3),sv.s);
                    end
                case 'w'
                    w = evt.AffectedObject.w;
                    if ~isequal(sv.w,w),
                        sv.w = w;
                        sv.xy = sv.w(1:2)*[-1 1];
                        x=-sv.w(1):sv.w(1); y=-sv.w(2):sv.w(2);
                        [sv.X, sv.Y]=meshgrid(x,y);
                    end
            end
        end
        
        function post(sv)
            sv.ecd = cross(sv.e,sv.cmrCen)/sv.d;
            if isequal(sv.e(1:2),[0 0]')
                sv.R = -eye(3);
            else
                ex = [-sv.e(2) sv.e(1) 0]'; ex = ex/norm(ex);
                ey = -cross(sv.e,ex); ey = ey/norm(ey);
                sv.R = [ex ey -sv.e];
            end
            sv.q = dcm2q(sv.R);
            sv.u = sv.cmr(:,1:3)*sv.e;
        end
        
        function rotate(sv)
            sv.R = q2dcm(sv.q);
            sv.Hr(:,1:2) = sv.d*sv.cmr(:,1:3)*sv.R(:,1:2);
            sv.re = [sv.ecd'*sv.R(:,1:2) sv.e'*sv.R(:,3)];
            sv.cr = sv.cmrCen'*sv.R(:,3)/sv.re(3);
            sv.vz = sv.v'*sv.R(:,3);
        end
        
        function elevate(sv)
            sv.v = sv.r*sv.e-sv.cmrCen;
            sv.Hr(:,3) = sv.r*sv.u+sv.cmr(:,4);
            sv.vz = sv.v'*sv.R(:,3);
        end
        
        function [Na,Nr,Nx,Ny,Nrx,Nry] = reset(sv,a,b,c)
            sv.d = 1;
            sv.r = 0; %RasterView.radiusMoon;
            sv.cmrCen = [0 0 sv.r+c]';
            sv.cmr = -[1 0 0; 0 1 0; a b 1];
            sv.cmr = [sv.cmr -sv.cmr*sv.cmrCen];
            sv.dd = sv.d^2*det(sv.cmr(:,1:3));
            sv.e = [0 0 1]';
            sv.post;
            sv.rotate;
            sv.elevate;
            sv.W(1:80,1:80) = checkerboard(20,2,2);
            U = a*sv.X+b*sv.Y+c;
            Na = U.^3/c;
            Nr = -U.^2.*(3*c-U)/c^2;
            Nx = 3*a*U.^2/c;
            Ny = 3*b*U.^2/c;
            Nrx = -3*a*U.*(2*c-U)/c^2;
            Nry = -3*b*U.*(2*c-U)/c^2;
        end
        
        function c = grad_se(sv,Wc,Ms,Ga,Gb)
            Gw = Wc.*(Ga-Ms); Mw = Gw.*(Ga+Ms);
            Gw = conv2(sv.s0,sv.s0,Gw,'same');
            Mw = conv2(sv.s0,sv.s0,Mw,'same');
            c = sv.Ja.*(Gb.*Gw-Mw/2);
        end
        
        function c = grad_ss(sv,Wc,Gb,Ms,Mx,My,Wt,Nx,Ny)
            Wcs = Wc./Wt; Wcs(isnan(Wcs)) = 0;
            gx = Mx.*Wcs; gy = My.*Wcs;
            s21 = conv2(sv.s0,sv.s1,gx,'same')+conv2(sv.s1,sv.s0,gy,'same');
            S22 = (gx.*Nx+gy.*Ny)./Wt; S22(isnan(S22)) = 0;
            s22 = conv2(sv.s0,sv.s0,S22,'same');
            
            s11 = conv2(sv.s0,sv.s1,gx.*Ms,'same')+conv2(sv.s1,sv.s0,gy.*Ms,'same');
            s12 = conv2(sv.s0,sv.s0,S22.*Ms,'same');
            
            S3 = (Mx.*gx+My.*gy)./Wt; S3(isnan(S3)) = 0;
            s3 = conv2(sv.s0,sv.s0,S3,'same')/2;
            c = sv.Ja.*(s11+s12-Gb.*(s21+s22)-s3);
        end
        
        function c = grad_ms(sv,Wc,Wt,Gb,Ms)
            Wcs = Wc./Wt; Wcs(isnan(Wcs)) = 0;
            Gc = conv2(sv.s0,sv.s0,Wcs,'same');
            Mc = conv2(sv.s0,sv.s0,Wcs.*Ms,'same');
            c = sv.Ja.*(Gb.*Gc-Mc);
        end
        
        function c = grad_mx(sv,Wc,Wt,Nx,Gb,Ms)
            gx = sv.grad_gx(Wc,Gb);
            
            % ms*wx
            Wcs = Nx.*Wc./Wt; Wcs(isnan(Wcs)) = 0;
            Gc = conv2(sv.s0,sv.s0,Wcs,'same');
            Mc = conv2(sv.s0,sv.s0,Wcs.*Ms,'same');
            ms = sv.Ja.*(Gb.*Gc-Mc);
            
            % ms*wx
            wx = -sv.Ja.*conv2(sv.s0,sv.s1,Wc.*Ms,'same');
            c = gx - ms - wx;
        end
        
        function c = grad_my(sv,Wc,Wt,Ny,Gb,Ms)
            gy = sv.grad_gy(Wc,Gb);
            
            % ms*wx
            Wcs = Ny.*Wc./Wt; Wcs(isnan(Wcs)) = 0;
            Gc = conv2(sv.s0,sv.s0,Wcs,'same');
            Mc = conv2(sv.s0,sv.s0,Wcs.*Ms,'same');
            ms = sv.Ja.*(Gb.*Gc-Mc);
            
            % ms*wx
            wy = -sv.Ja.*conv2(sv.s1,sv.s0,Wc.*Ms,'same');
            c = gy - ms - wy;
        end
        
        function c = grad_gs(sv,Wc,Gb)
            c = sv.Ja.*Gb.*conv2(sv.s0,sv.s0,Wc,'same');
        end
        
        function c = grad_gx(sv,Wc,Gb)
            c = -sv.Ja.*Gb.*conv2(sv.s0,sv.s1,Wc,'same');
        end
        
        function c = grad_gy(sv,Wc,Gb)
            c = -sv.Ja.*Gb.*conv2(sv.s1,sv.s0,Wc,'same');
        end
        
        function c = grad_ws(sv,Wc)
            c = sv.Ja.*conv2(sv.s0,sv.s0,Wc,'same');
        end
        
        function c = grad_wx(sv,Wc)
            c = -sv.Ja.*conv2(sv.s0,sv.s1,Wc,'same');
        end
        
        function c = grad_wy(sv,Wc)
            c = -sv.Ja.*conv2(sv.s1,sv.s0,Wc,'same');
        end
        
        function c = grad_nt(sv,c)
            c = sv.Ja.*conv2(sv.s0,sv.s0,c,'same');
        end
        
        function c = grad_ns(sv,Wc,Nr,Wt)
            % Wc: correlation winodw
            % Nr: signal dof
            % Wt: total dof
            c = Wc.*(2*sv.Na-Nr)./Wt; c(isnan(c)) = 0;
            c = sv.Ja.*conv2(sv.s0,sv.s0,c,'same');
        end
        
        function set.h(obj,h)
            % h property set function
            if length(h) == 3
                obj.h = h;
            else
                error('h must be a 3-vector')
            end
        end % set.h
        
        function h = disp(sv,h)
            if nargin < 2, h = figure; else figure(h), end
            imshow(sv.img)
        end
        
        function [Fa,Na,Fx,Nx,Fy,Ny,Fk,Nk] = crop(sv)
            sv.proj; sv.smooth;
            if nargout > 0, Fa = sv.Fa; end
            if nargout > 1, Na = sv.Na; end
            if nargout > 2, Fx = sv.Fx; end
            if nargout > 3, Nx = sv.Nx; end
            if nargout > 4, Fy = sv.Fy; end
            if nargout > 5, Ny = sv.Ny; end
            if nargout > 6, Fk = sv.Fk; end
            if nargout > 7, Nk = sv.Nk; end
        end
        
        function [Gr,Nr,Grx,Nrx,Gry,Nry] = grad_r(sv)
            [sv.Nr, sv.Nrx, sv.Nry] = sv.dGdR(sv.W, sv.Wa,sv.Wx,sv.Wy);
            [sv.Gr, sv.Grx, sv.Gry] = sv.dGdR(sv.Gk,sv.Fa,sv.Fx,sv.Fy);
           
            if nargout > 0,  Gr = sv.Gr; end
            if nargout > 1,  Nr = sv.Nr; end
            if nargout > 2, Grx = sv.Grx; end
            if nargout > 3, Nrx = sv.Nrx; end
            if nargout > 4, Gry = sv.Gry; end
            if nargout > 5, Nry = sv.Nry; end
        end
        
        function [Gr,Grx,Gry] = dGdR(sv,Fk,Fa,Fx,Fy)
            Fxy = conv3(Fk,sv.s1,sv.s1);
            [Fxx,Fyy] = conv3xy(Fk,sv.s0,sv.s2);

            Fr  = deriv( Fx, Fy,sv.X,sv.Y,sv.re,sv.vz);
            Frx = deriv(Fxx,Fxy,sv.X,sv.Y,sv.re,sv.vz);
            Fry = deriv(Fxy,Fyy,sv.X,sv.Y,sv.re,sv.vz);
            
            Gr  = sv.Jr.*Fa+sv.Ja.*Fr;
            Grx = sv.Jrx.*Fa+sv.Jx.*Fr+sv.Jr.*Fx+sv.Ja.*Frx;
            Gry = sv.Jry.*Fa+sv.Jy.*Fr+sv.Jr.*Fy+sv.Ja.*Fry;
        end
    end % methods
    
    methods (Access = private)
        function proj(sv)
            sv.tF = maketform('projective',(sv.Hs/sv.Hr)');
            % backward projection of image and weight
            sv.Fk = imtransform(sv.img,sv.tF,'bicubic',...
                'xdata',sv.xy(1,:),'ydata',sv.xy(2,:));
            
            % determinant of backward Jacobian
            i3 = sv.Hr(3,1)*sv.X+sv.Hr(3,2)*sv.Y+sv.Hr(3,3);
            n = sv.dd*sv.vz;
            d2 = i3.^2/n;
            sv.Ja = i3.^3/n;
            h = sv.r-sv.cr;
            i3z = i3/h;
%            sv.Jr = d2.*(3*sv.u(3)-i3z);
            sv.Jr = 3*sv.u(3)*d2-sv.Ja/h;
            a = 3*sv.Hr(3,1); b = 3*sv.Hr(3,2);
            sv.Jx = a*d2; sv.Jy = b*d2;
            sv.Jrx = a*i3.*(2*sv.u(3)-i3z)/n;
            sv.Jry = b*i3.*(2*sv.u(3)-i3z)/n;

            D = sv.Hr(3,3)^3/det(sv.Hr); % determinant of jacobian
            if (sv.Ja((1+end)/2)-D) > SingleView.eps_sqrt
                figure, imagesc(sv.Ja), colorbar
                error('Jacobian should be similar to numerical estimate!!!');
            end

            if any(sv.Ja(:) < 0),
                sv.R, sv.q, sv.r
                figure, imagesc(sv.Ja), colorbar
                fprintf('Weight should be nonnegative!!!')
            end
        end
        
        function smooth(sv)
            sv.Nk = sv.W.*sv.Ja; sv.Gk = sv.W.*sv.Fk;
            [sv.Wa,sv.Wx,sv.Wy] = conv3(sv.W,sv.s0,sv.s1);
            sv.Na = sv.Ja.*sv.Wa;
            sv.Nx = sv.Jx.*sv.Wa+sv.Ja.*sv.Wx;
            sv.Ny = sv.Jy.*sv.Wa+sv.Ja.*sv.Wy;
            [sv.Fa,sv.Fx,sv.Fy] = conv3(sv.Gk,sv.s0,sv.s1);
            sv.Ga = sv.Ja.*sv.Fa;
            sv.Gx = sv.Jx.*sv.Fa+sv.Ja.*sv.Fx;
            sv.Gy = sv.Jy.*sv.Fa+sv.Ja.*sv.Fy;
        end        
    end
    
    methods (Static)
    end % methods (Access = private)
end % classdef

function [Fa,Fx,Fy] = conv3(G,s0,s1)
Fa=conv2(s0,s0,G,'same');
if nargout > 1, [Fx,Fy]=conv3xy(G,s0,s1); end
end

function [Fx,Fy] = conv3xy(G,s0,s1)
Fx=conv2(s0,s1,G,'same');
Fy=conv2(s1,s0,G,'same');
end

function Fr = deriv(Fx,Fy,X,Y,re,vz)
Fr = -(re(3)*(Fx.*X+Fy.*Y)-re(2).*Fx+re(1).*Fy)/vz;
end

function [f0 f1 f2]=fltGaussian(r,s,c)
% r: radius, s: sigma
if nargin < 3, c = 0; end
x=[-r-0.5:r+0.5]'+c; 
fx = normpdf(x,0,s); fx(1) = 0; fx(end) = 0;
Fx = normcdf(x,0,s); Fx(1) = 0; Fx(end) = 1;
f0 = diff(Fx); f1 = diff(fx);
f2 = diff(-x.*fx)/s^2;
end
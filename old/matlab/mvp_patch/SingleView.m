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
        
        Ib, Wb  % (backward) projected image and weight
        Is, Ws  % smoothed weighted image and weight
        Ix, Wx  % gradient weighted image and weight
        Iy, Wy  % gradient weighted image and weight
        It      % gradient w.r.t theta
        Wt      % =W*Wb
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
                        sv.Wb = ones(size(sv.X)); sv.W = sv.Wb/2;
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
                    sv.Wt = sv.W.*sv.Wb;
                    [sv.Ws,sv.Wx,sv.Wy] = smooth(sv.Wt,sv.s0,sv.s1);
            end
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
                error('W must be a probability')
            else
                obj.W = W;
            end
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
        
        function [Is,Ws,Ix,Wx,Iy,Wy,Ib,W] = crop(sv)
            sv.proj;
            if nargout > 0, Is = sv.Is; end
            if nargout > 1, Ws = sv.Ws; end
            if nargout > 2, Ix = sv.Ix; end
            if nargout > 3, Wx = sv.Wx; end
            if nargout > 4, Iy = sv.Iy; end
            if nargout > 5, Wy = sv.Wy; end
            if nargout > 6, Ib = sv.Ib; end
            if nargout > 7, W = sv.W; end
        end
    end % methods
    
    methods (Access = private)
        function [Is,Ws,Ix,Wx,Iy,Wy] = proj(sv)
            % backward projection of image and weight
            tform = maketform('projective',inv(sv.H)');
            sv.Ib = imtransform(sv.img,tform,'bicubic','xdata',sv.xb,'ydata',sv.yb);
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
            
            [sv.Ws,sv.Wx,sv.Wy] = smooth(sv.Wt,sv.s0,sv.s1);
            [sv.Is,sv.Ix,sv.Iy] = smooth(sv.Wt.*sv.Ib,sv.s0,sv.s1);
            
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
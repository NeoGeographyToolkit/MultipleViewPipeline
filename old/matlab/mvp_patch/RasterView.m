classdef RasterView < handle
    % Raster
    properties (Constant)
        radiusMoon = 1737400;   % radius of the moon
    end % properties (Constant)
    
    properties (SetObservable)
        z = [1 1]';     % point of interest
    end
    
    properties (Hidden)
        pv = PatchViews;
        sv = SingleView;
        tv = TerainView;
        lonlat          % longitude and latitude
    end
    
    methods
        function rv = RasterView(sv,tv)
            if nargin == 2,
                rv.tv=tv; rv.sv=sv;
                addlistener(rv,'z','PostSet',@rv.PropEvents);
                rv.pv = PatchViews(sv);
                % initialize RasterView objects
                rv.z = [1 1]';
            end
        end
        
        function h = corelate(rv,z)
            if nargin > 1,
                rv.z = z;
            end
            rv.pv.proj
            r = rv.elevate(RasterView.radiusMoon-500);
            h = r-RasterView.radiusMoon
            q = rv.rotate
        end

        function h = raster(rv,z)
            if nargin > 1,
                rv.z = z;
            end
            rv.pv.proj
            r = rv.elevate(RasterView.radiusMoon-500);
            h = r-RasterView.radiusMoon
            
            q = rv.rotate
            for k=1:5
                k
                [p(k),~,~,f(k)] = rv.pv.corelate;
                t = rv.pv.geometry
                rv.pv.proj
            end
            figure, plot(log(f))
            figure, plot(log(p))
            rv.disp
            t = rv.scate
            s = rv.smote
            for k=1:5
                k
                [p(k),~,~,f(k)] = rv.pv.corelate;
                t = rv.pv.geometry
                rv.pv.proj
            end
            figure, plot(log(f))
            figure, plot(log(p))
            c = rv.pv.robust
            rv.disp
        end

        function h = proj(rv,z)
            if nargin > 1,
                rv.z = z;
            end
            rv.pv.proj
        end

        function r=elevate(rv,r)
            if nargin > 1,
                r=rv.pv.elevate(r);
            else
                r=rv.pv.elevate;
            end
        end
        
        function q=rotate(rv)
            q=rv.pv.rotate;
        end
        
        function s=scate(rv)
            s=rv.pv.scate.^2;
        end
        
        function s=smote(rv)
            s=rv.pv.smote.^2;
        end
        
        function c = slote(rv)
            c = rv.pv.slote;
        end
        
        function s = sidate(rv)
            s = rv.pv.sidate.^2;
        end
        
        function h = disp(rv)
            fprintf('\npoint of interest (%d,%d) in the %d*%d tile',rv.z,rv.tv.size)
            fprintf('\nlongitude and latitude (%d,%d) \n',rv.lonlat*180/pi)
            rv.pv.disp
        end
        
        function PropEvents(rv,src,evt)
            switch src.Name
                case 'z'
                    [rv.pv.e, ~, rv.lonlat]=rv.tv.post(rv.z);
            end
        end
        
        function set.z(obj,z) % z property set function
            obj.z = z;
        end % set.z
        
        function z=get.z(obj) % z property set function
            z = obj.z;
        end % get.z
    end % methods
    
end % classdef

function [f0 f1 f2]=fncGaussian(r,s)
% r: radius, s: sigma
x=[-r-0.5:r+0.5]'; fx = normpdf(x,0,s);
f0 = diff(normcdf(x,0,s));
f1 = diff(fx);
f2 = diff(-x.*fx)/s^2;
end

function [w0 w1 w2]=wndGaussian(r,s)
% r: radius, s: sigma
[w0 w1 w2]=fncGaussian(r,s);
n0 = sum(w0);
w1 = -w1*s^2/n0;
w2 = s^2*(s^2*w2+w0)/n0;
w0 = w0/n0;
end

function [S,R] = scatw(I,C,K,gx,gy)
% asymmetric scatter
c = wnd3(gy,gx,I);
t = wnd3(gy,gx,C);
R = c(K);
S = diag(t)-R;
end

function b = wnd3(hcol,hrow,A,sz)
% 3-dimensional separable windowing
% b = hcol'*reshape(A,sz(1),prod(sz(2:end)));
% b = hrow'*reshape(b,sz(2),sz(end));
C = hcol*hrow'; if nargin < 4, sz = size(A); end
b = (C(:)'*reshape(A,sz(1)*sz(2),prod(sz(3:end))))';
end

function stop=mvOutFcn(x,optimvalues,state)
if optimvalues.fval < RasterView.eps_log2p,
    stop = true;
else
    stop = false;
end
end

function [p,f] = fpval(ss,se,ns,ne,eps_p)
if nargin<5, eps_p = realmin; end
if se > 0
    if ss > 0, x = se/(se+ss); else x = 1; end
else
    if ss > 0, x = 0; else x = 0.5; end
end
if ne < eps_p, ne=eps_p; end
if ns < eps_p, ns=eps_p; end
p = betainc(x,ne,ns);
if nargout > 1, f = ss*ne/(se*ns); end
end

function [p,p1,p2] = fbval(ss,se,ns,ne,eps_p)
if nargin<5, eps_p = realmin; end
p1 = fpval(ss,se,ns,ne,eps_p);
p2 = fpval(ns^2*se,ne^2*ss,ns,ne,eps_p);
p = abs(p1-p2);
end
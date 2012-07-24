classdef MultiViews < handle
    properties (Hidden)
        strPath        % directory
        sv = SingleView;
        pv = PatchViews;
        tv = TerainView;
        opt;
    end
    
    properties (Constant)
        log_eps = log(eps(single(0)));
    end % properties (Constant)
    
    properties (Dependent, SetAccess = private)
    end % properties Dependent = true, SetAccess = private
    
    methods
        function mv = MultiViews(str)
            mv.strPath = str;
            load(str);
            n = numel(images);
            mv.tv = TerainView(tile_georef.transform,tile_size);
            
            for k=n:-1:1
                mv.sv(k)=SingleView(images(k).data/2^17,images(k).camera);
            end
            mv.pv = PatchViews(mv.sv,mv.tv); % = PatchViews(zeros(0,0,mv.n),ones(0,0,mv.n));
            
            mv.opt.R = optimset('disp','iter','FunValCheck','off');
        end
        
        function h = disp(mv)
            n = numel(mv.sv);
            for k=1:n, mv.sv(k).disp; end
            mv.pv.disp;
            mv.tv.disp;
        end
        
        function proj(mv,z)
            if nargin > 1, mv.pv.z = z; end
            mv.pv.proj
        end
        
        function r=elevate(mv,r)
            if nargin > 1,
                r=mv.pv.elevate(r);
            else
                r=mv.pv.elevate;
            end
        end
        
        function q=rotate(mv)
            q=mv.pv.rotate;
        end
        
        function s=scate(mv)
            s=mv.pv.scate.^2;
        end
        
        function s=smote(mv)
            s=mv.pv.smote.^2;
        end
        
        function c = slote(mv)
            c = mv.pv.slote;
        end
        
        function s = sidate(mv)
            s = mv.pv.sidate.^2;
        end
        
        function r = profile(mv,lb,ub,step)
            if nargin < 2, lb = mv.pv.r-2000; end
            if nargin < 3, ub = mv.pv.r+2000; end
            if nargin < 4, step = 30; end
            R = [lb:step:ub];
            E = []; fprintf('%d: ', length(R));
            for r = R,
                mv.pv.r=r; E = [E mv.corelate]; fprintf('.');
            end
            fprintf('\n');
            
            r = R-PatchViews.radiusMoon;
            D = log(E);
            figure,
            subplot(1,2,2), plot(r,E,'-k'),
            axis([min(r) max(r) 0 1]), title('p-value', 'FontSize', 14)
            subplot(1,2,1), plot(r,D,'-k')
            minD =min(D); maxD = max(D);
            if isfinite(minD) && isfinite(maxD),
                axis([min(r) max(r) minD maxD])
            end
            title('Log p-value', 'FontSize', 14)
            figure, plot(r,D,'-r'),
            title('Log p-value', 'FontSize', 14)
        end
        
        function r = profile_slp(mv,lb,ub,step)
            if nargin < 2, lb = mv.pv.c-10; end
            if nargin < 3, ub = mv.pv.c+10; end
            if nargin < 4, step = .2; end
            S = [lb:step:ub];
            E = []; fprintf('%d: ', length(S));
            for s = S,
                p=mv.slopate(s);
                E = [E; p]; fprintf('.');
            end
            fprintf('\n');
            
            D = log(E);
            figure,
            subplot(1,2,2), plot(S,E,'-k'),
            axis([min(S) max(S) 0 1]), title('p-value', 'FontSize', 14)
            subplot(1,2,1), plot(S,D,'-k')
            minD =min(D); maxD = max(D);
            if isfinite(minD) && isfinite(maxD),
                axis([lb ub minD maxD])
            end
            title('Log p-value', 'FontSize', 14)
            figure, plot(S,D,'-r'),
            title('Log p-value', 'FontSize', 14)
        end
        
        function r = profile_scx(mv,lb,ub,step)
            if nargin < 2, lb = 0; end
            if nargin < 3, ub = 1.5*max(mv.pv.t); end
            if nargin < 4, step = max(mv.pv.t)/10; end
            %            mv.pv.adjustHypothesis;
            t = mv.pv.t;
            c = 1.5; d = 2; r = 1/30;
            step = sqrt(prod(t))*r;
            X = [t(1)/d:step:c*t(1)];
            Y = [t(2)/d:step:c*t(2)];
            P = []; Q = []; fprintf('%d*%d: ', length(X),length(Y));
            
            for x = X,
                a = []; b = [];
                fprintf('%5.2f',x);
                for y = Y
                    %                    mv.pv.t(3) = mv.inflate();
                    mv.pv.t=[x y]';
                    p = mv.corelate;
                    q = mv.correlate;
                    a = [a p]; b = [b q];
                    fprintf('.');
                end
                P = [P; a];
                Q = [Q; b];
                fprintf('\n');
            end
            fprintf('\n');
            
            fprintf('\n');
            D = log(P);
            figure,
            surf(Y,X,D), shading interp, colorbar, view(0,90)
            axis equal, title('Log p-value of signal to noise', 'FontSize', 14)
            figure,
            surf(Y,X,P), shading interp, colorbar, view(0,90)
            axis equal, title('p-value of signal to noise', 'FontSize', 14)
            figure,
            surf(Y,X,log(Q)), shading interp, colorbar, view(0,90)
            axis equal, title('Log p-value of signal to noise', 'FontSize', 14)
            figure,
            surf(Y,X,Q), shading interp, colorbar, view(0,90)
            axis equal, title('p-value of signal to noise', 'FontSize', 14)
            figure,
            plot(diag(D));
            
            mv.pv.t = t;
            ub = 5*mv.pv.s;
            step = mv.pv.s/30;
            S = [step:step:ub];
            
            a = [];
            for x = S,
                mv.pv.s=x;
                p = mv.corelate;
                a = [a p];
                fprintf('.');
            end
            figure, plot(S,a,'r')
            figure, plot(S,log(a),'r')
        end
        
        function [p,a,b]=corelate(mv)
            mv.pv.proj;
            [p,a,b]=mv.pv.corelate;
        end
        
        function [p,a,b]=correlate(mv)
            mv.pv.proj;
            [p,a,b]=mv.pv.correlate;
        end
        
        function c=inflate(mv)
            c=mv.pv.inflate;
        end
        
    end % methods
end % classdef

% properties
% q: quaternion
% r: radial elevation
% s: smoothing size
% t: window size

% operations
% initialize q, r, s, t
% compute confidence
% optimize q, r, s, t
% display patches

% status of views
% rotated -
% translated
% number of overlap
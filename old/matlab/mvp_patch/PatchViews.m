classdef PatchViews < handle
    % terrain patch contains handle of single view
    properties (Constant)
        ratioScale = 4;
        eps_realmin = realmin;
        eps_p = realmin;
        eps_log2p = reallog(2*realmin);
        eps_log = reallog(realmin);
        eps_rescale = eps;
        max_iter = 100;
        t0 = 1*[1 1]';
    end % properties (Constant)
    
    properties (SetObservable)
        n = 2;          % number of patches
        r = 0;          % radial elevation
        e = [0 0 1]     % elevation post
        q = [0 0 0 1]'; % quaternion of rotation
        s = 0.5;        % smoothing scale
        t = [10 10]';   % correlation scale
        u = 31;         % unit length of the pixel
        w = [40 40 3]'-1; % window size
        H               % sloppy refinement
        W               % inlier membership
    end
    
    properties (Hidden)
        sv = SingleView;
        Fa, Fb, Fn, Fs, Fx, Fy,
        Ma, Gb, Ga, Gx, Gy, Gt, Gtx, Gty, G2      % albedo gradients
        Fr, Frx, Fry, Gr, Grt, Grx, Gry, Grtx, Grty, Gz
        Mm, Mx, My, Mr, Mrx, Mry
        Es, Eb  % albedos
        Nb, Na, Nx, Ny
        Nr, Nrs, Nrx, Nry, Nrt, Ntx, Nty, Nrtx, Nrty
        NT, Nt, Nn, Ns      % total and normalized weight
        Wr0, Gr0, Mr0
        Ic, Wc          % corrected by linear reflectance
        n2, N, nc, is, js, ks, ia, ja, ka,
        x0, x1, x2, y0, y1, y2,
        gs, gx, gy,
        ms, mx, my
        ws, wx, wy  % to verify the gradient
        ls, lx, ly,
        Ds              % disparity
        rof, sof   % reciprocals of dof
        ne, nr, ns, nt  % degrees of freedom
        se, ss          % squared error and signal
        a, b;     % reflectance coefficients
        d = [0 32];         % shape parameters of exponetial power function
        m = 1;          % multiplicative hypothesis
        p               % p-value of correspondences
        f, df           % squared error
        X, Y, Z
        sw              % individual total dofs
        dw = [40 40]'   % increment of w(1:2)
        
        opt;            % optimization settings
    end
    
    methods
        function pv = PatchViews(sv,e)
            if nargin == 0, return; end
            addlistener(pv,'e','PostSet',@pv.PropEvents);
            addlistener(pv,'n','PostSet',@pv.PropEvents);
            addlistener(pv,'t','PostSet',@pv.PropEvents);
            addlistener(pv,'H','PostSet',@pv.PropEvents);
            addlistener(pv,'W','PostSet',@pv.PropEvents);
            pv.sv=sv; pv.n=numel(sv);
            pv.H = zeros(pv.n,3);
            for i=1:pv.n,
                obj = sv(i);
                addlistener(pv,'e','PostSet',@obj.PropEvents);
                addlistener(pv,'q','PostSet',@obj.PropEvents);
                addlistener(pv,'r','PostSet',@obj.PropEvents);
                addlistener(pv,'s','PostSet',@obj.PropEvents);
                addlistener(pv,'w','PostSet',@obj.PropEvents);
            end
            
            % initialize SingleView objects
            if nargin > 1, pv.e = e; end
            pv.a = ones(pv.n,1);
            pv.b = zeros(pv.n,1);
            pv.s = pv.s;
            pv.t = pv.t;
            pv.r = RasterView.radiusMoon;
            pv.W = ones(size(pv.X)); % convn(rand(size(pv.X)),ones(5),'same');
            
            pv.opt.R = optimset('disp','iter','Largescale','off');
            pv.opt.S = optimset(pv.opt.R,'MaxIter',1,'Algorithm','interior-point');
            pv.opt.W = optimset(pv.opt.R,'MaxIter',0,'TolX',1e-25);
            pv.opt.W = optimset(pv.opt.W,'GradObj','off');
            pv.opt.T = optimset('Largescale','on','FinDiffType','central');
            pv.opt.T = optimset(pv.opt.T,'MaxIter',0,'DerivativeCheck','on');
            pv.opt.T = optimset(pv.opt.T,'GradObj','on','OutputFcn',@outfun);
            pv.opt.G = optimset(pv.opt.R,'GradObj','on','DerivativeCheck','off');
        end
        
        function m = adjustHypothesis(pv)
            pv.corelate;
            pv.m=pv.m*pv.f;
            if nargout > 0, m = pv.m; end
        end
        
        function r = elevate(pv,r)
            if nargin > 1, pv.r=r; end
            for k=1:PatchViews.max_iter
                [r,f,exitflag,output] = fminunc(@(r)mvOpt(r,pv),pv.r,pv.opt.R);
                if f < PatchViews.eps_log2p,
                    pv.r=r; pv.proj;
                    m = pv.adjustHypothesis;
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('H0: var_s = %f var_e !!!\n',m);
                    end
                else
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('%dth iteration for hypothetical adjustment\n',k);
                    end
                    break,
                end
            end
            pv.r=r; pv.proj;
            if ~isequal(pv.opt.R.Display,'off'),
                %                pv.disp;
            end
            
            function p=mvOpt(r,pv)
                pv.r=r; pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
        end
        
        function q = rotate(pv)
            for k=1:PatchViews.max_iter
                [q,f,exitflag,output] = fminunc(@(q)mvOpt(q,pv),pv.q,pv.opt.R);
                if f < PatchViews.eps_log2p,
                    pv.q=q; pv.proj;
                    m = pv.adjustHypothesis;
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('H0: var_s = %f var_e !!!\n',m);
                    end
                else
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('%dth iteration for hypothetical adjustment\n',k);
                    end
                    break,
                end
            end
            pv.q=q; pv.proj;
            if ~isequal(pv.opt.R.Display,'off'),
                output
                exitflag
                pv.disp
            end
            
            function p=mvOpt(q,pv)
                pv.q=q; pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
        end
        
        function t = scate(pv)  % optimize the scales
            t = sqrt(pv.t);
            for k=1:PatchViews.max_iter
                [t,f,exitflag,output] = fminunc(@(t)mvOpt(t,pv),t,pv.opt.R);
                if f < PatchViews.eps_log2p,
                    pv.t = t.^2; pv.proj;
                    m = pv.adjustHypothesis;
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('H0: var_s = %f var_e !!!\n',m);
                    end
                else
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('%dth iteration for hypothetical adjustment\n',k);
                    end
                    break,
                end
            end
            pv.t = t.^2; pv.proj;
            if ~isequal(pv.opt.R.Display,'off'),
                output
                exitflag
                pv.disp
            end
            function [f,p]=mvOpt(t,pv)
                pv.t = t.^2; pv.proj;
                p=pv.corelate;
                if p<realmin, f = PatchViews.eps_log; else f = log(p); end
            end
        end
        
        function t = scale(pv)  % optimize the scales
            w = pv.w(1:2);
            pv.c = pv.w(1:2)+pv.dw;
            ub = sqrt(pv.c/PatchViews.ratioScale);
            t = fmincon(@(t)mvOpt(t,pv),sqrt(pv.t),[],[],[],[],[0 0],ub,[],pv.opt.S);
            pv.t = t.^2; pv.proj;
            dw = pv.w(1:2)-w;
            pv.dw = ceil(abs(dw).*2.^sign(dw))+1;
            pv.c = pv.w(1:2);
            if ~isequal(pv.opt.R.Display,'off'),
                pv.disp
            end
            
            function p=mvOpt(t,pv)
                pv.t = t.^2;
                pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
        end
        
        function s = smote(pv)  % optimize the scales
            s = sqrt(pv.s);
            for k=1:PatchViews.max_iter
                [s,f,exitflag,output] = fminunc(@(s)mvOpt(s,pv),s,pv.opt.R);
                if f < PatchViews.eps_log2p,
                    pv.s = s^2;
                    pv.proj;
                    m = pv.adjustHypothesis;
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('H0: var_s = %f var_e !!!\n',m);
                    end
                else
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('%dth iteration for hypothetical adjustment\n',k);
                    end
                    break,
                end
            end
            pv.s = s^2;
            pv.proj;
            if ~isequal(pv.opt.R.Display,'off'),
                output
                exitflag
                pv.disp
            end
            function [f,p]=mvOpt(s,pv)
                pv.s = s^2;
                pv.proj;
                p=pv.corelate;
                if p<realmin, f = PatchViews.eps_log; else f = log(p); end
            end
        end
        
        function s = sidate(pv)  % optimize the scales
            s = prod(pv.t)^0.25;
            for k=1:PatchViews.max_iter
                [s,f,exitflag,output] = fminbnd(@(s)mvOpt(s,pv),s/2,2*s,pv.opt.R);
                %                [s,f,exitflag,output] = fminunc(@(s)mvOpt(s,pv),s,pv.opt.R);                [f,p]=mvOpt(s,pv);
                if f < PatchViews.eps_log2p,
                    pv.t=s([1;1]).^2; pv.proj;
                    m = pv.adjustHypothesis;
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('H0: var_s = %f var_e !!!\n',m);
                    end
                else
                    if ~isequal(pv.opt.R.Display,'off'),
                        fprintf('%dth iteration for hypothetical adjustment\n',k);
                    end
                    break,
                end
            end
            %            pv.s = s(2)^2; pv.t=s([1;1]).^2; pv.proj;
            pv.t=s([1;1]).^2; pv.proj;
            if ~isequal(pv.opt.R.Display,'off'),
                output
                exitflag
                pv.disp
            end
            function [f,p]=mvOpt(s,pv)
                pv.t=s([1;1]).^2; pv.proj;
                p=pv.corelate;
                if p<realmin, f = PatchViews.eps_log; else f = log(p); end
            end
        end
        
        function [r,q] = optimize(pv,r,q)
            if nargin > 1, pv.r = r; end
            if nargin > 2, pv.q = q; end
            [r0,k] = pv.ggnElevate;
            r = pv.elevate(pv.r);
            h0 = r0-RasterView.radiusMoon;
            h = r-RasterView.radiusMoon;
            fprintf('The elevation is %f with %d iterations - %f = %f',h0,k,h,h-h0);
            
            r = pv.r; q = pv.q;
            %             pv.opt.Q = optimset(pv.opt.R,'OutputFcn',@rotfun);
            %             pv.opt.R = optimset(pv.opt.R,'disp','off','OutputFcn',@radfun);
            %             pv.opt.S = optimset(pv.opt.S,'disp','off');
            %             [q,f,exitflag,output] = fminunc(@(q)mvOpt(q,pv),q,pv.opt.Q);
            %             pv.q=q; pv.proj;
            %             if ~isequal(pv.opt.Q.Display,'off'),
            %                 output
            %                 exitflag
            %                 pv.disp
            %             end
            
            function p=mvOpt(q,pv)
                pv.q=q; pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
            
            function stop = rotfun(q,optimvalues,state)
                stop = false;
                switch state
                    case 'iter'
                        pv.q = q; pv.proj;
                        r = pv.elevate;
                        h = r-RasterView.radiusMoon;
                        fprintf('The elevation is %f\n', h);
                end
            end
            
            function stop = radfun(r,optimvalues,state)
                stop = false;
                switch state
                    case 'iter'
                        pv.r = r; pv.proj;
                        pv.geometry;
                end
            end
        end
        
        function h = disp(pv)
            if ~isempty(pv.W)
                figure;
                C = num2cell(pv.W,[1 2]);
                C = reshape(C,[2 2]);
                C = cell2mat(C);
                figure, imagesc(C), axis equal, colorbar;
            end
            
            if ~isempty(pv.Fa)
                pv.residual;
                A = num2cell(pv.Fa,[1 2]);
                A = reshape(A,[2 2]);
                A = cell2mat(A);
                
                h = figure;
                subplot(2,3,1), imshow(imadjust(A)), axis equal
                title('Before Photometric Correction','FontSize',14)
                
                C = num2cell(pv.Ma,[1 2]);
                C = reshape(C,[2 2]);
                C = cell2mat(C);
                subplot(2,3,2), imagesc(C), axis off equal
                title('After Photometric Correction','FontSize',14)
                E = std(pv.Es,0,3);
                
                D = num2cell(pv.Es,[1 2]);
                D = reshape(D,[2 2]);
                D = cell2mat(D);
                subplot(2,3,5), imagesc(D), axis off equal
                title('Residual of Image Patches','FontSize',14)
                
                subplot(2,3,6), imagesc(pv.G2), axis off equal
                title('Magnitude of Gradient Albedo','FontSize',14)
                
                subplot(2,3,3), imagesc(pv.Mm), axis off equal
                title('Estimated Albedo','FontSize',14)
                
                subplot(2,3,4), imagesc(E), axis off equal
                title('Standard Error','FontSize',14)
            end
        end
        
        function PropEvents(pv,src,evt)
            switch src.Name
                case 'e'
                    ex = [-pv.e(2) pv.e(1) 0]'; ex = ex/norm(ex);
                    ey = -cross(pv.e,ex); ey = ey/norm(ey);
                    R = [ex ey -pv.e];
                    pv.q = dcm2q(R)';
                case 'n'
                    pv.initIndices;
                case 't'
                    pv.initWindows;
                case 'H'
                    H = evt.AffectedObject.H;
                    for k=1:pv.n,
                        if ~isequal(pv.sv(k).h',H(k,:)),
                            pv.sv(k).h = H(k,:)';
                        end
                    end
                case 'W'
                    W = evt.AffectedObject.W;
                    for k=1:pv.n,
                        if ~isequal(pv.sv(k).W,W(:,:,k)),
                            pv.sv(k).W = pv.W(:,:,k);
                        end
                    end
            end
        end
        
        function initWindows(pv)
            w = ceil([pv.t(1:2)*PatchViews.ratioScale; pv.s*SingleView.ratioSmoth]);
            if ~isequal(pv.w,w),
                pv.w = w;
                [pv.X, pv.Y pv.Z]=meshgrid(-w(1):w(1),-w(2):w(2),1:pv.n);
                pv.W = ones(size(pv.X)); % convn(rand(size(pv.X)),ones(5),'same');
            end
            [pv.x0 pv.x1 pv.x2] = wndGaussian(pv.w(1),pv.t(1));
            [pv.y0 pv.y1 pv.y2] = wndGaussian(pv.w(2),pv.t(2));
            sof=diff(normcdf([-0.5 0.5],0,pv.s))^2; % smoothing compensation
            cof = pv.x0(w(1)+1)*pv.y0(w(2)+1);      % central weight dof
            pv.rof = sof/cof/2;                     % half ratio of dofs
        end
        
        function initIndices(pv)             % derived parameters from n
            pv.n2 = pv.n^2; pv.N = [1:pv.n]';
            C = [[pv.N pv.N]; nchoosek(pv.N,2)];
            pv.is = C(:,1); pv.js = C(:,2);
            idx = sub2ind([pv.n pv.n],pv.is,pv.js);
            pv.ks = zeros(pv.n); pv.ks(idx) = 1:length(idx);
            idx = sub2ind([pv.n pv.n],pv.js,pv.is);
            pv.ks(idx) = 1:length(idx);
            pv.nc = nchoosek(pv.n,2)+pv.n;
            
            % augmented for 1st order scatter matrix
            pv.ia = [C(:,1); C(pv.n+1:end,2)];
            pv.ja = [C(:,2); C(pv.n+1:end,1)];
            idx = sub2ind([pv.n pv.n],pv.ia,pv.ja);
            pv.ka = zeros(pv.n); pv.ka(idx) = 1:length(idx);
        end
        
        function [I,W]=proj(pv)
            pv.Fa=[]; pv.Fx=[]; pv.Fy=[]; pv.Fb=[];
            pv.Na=[]; pv.Nx=[]; pv.Ny=[]; pv.Nb=[];
            for k=1:pv.n
                [pv.Fa(:,:,k),pv.Na(:,:,k),pv.Fx(:,:,k),pv.Nx(:,:,k),...
                    pv.Fy(:,:,k),pv.Ny(:,:,k),pv.Fb(:,:,k),pv.Nb(:,:,k)] = pv.sv(k).crop;
            end
            
            pv.Ma = pv.Fa./pv.Na; pv.Ma((isnan(pv.Ma))) = 0;
            pv.Nt = sum(pv.Na,3); pv.NT = repmat(pv.Nt,[1 1 pv.n]);
            pv.Nn = pv.Na./pv.NT; pv.Nn(isnan(pv.Nn)) = 0;
            pv.Fn = pv.Fa./pv.NT; pv.Fn(isnan(pv.Fn)) = 0;
            pv.Ns = sum(pv.Nn.*pv.Na,3);
            
            if nargout > 0, I = pv.Fa; end
            if nargout > 1, W = pv.Na; end
        end
        
        function E = residual(pv)
            pv.Gb = pv.a(pv.Z).*pv.Fb+pv.b(pv.Z);
            pv.Ma = pv.Ga./pv.Na; pv.Ma((isnan(pv.Ma))) = 0;
            pv.Mm = sum(pv.Ga,3)./pv.Nt; pv.Mm((isnan(pv.Mm))) = 0;
            pv.Es = pv.Gb - pv.Mm(:,:,ones(pv.n,1));
            Nt = sum(pv.W.*pv.Nb,3); Gb = sum(pv.Gb,3);
            Mb = Gb./Nt; Mb(isnan(Mb)) = 0;
            Mm = (pv.Nt.*pv.Mm+Nt.*Mb)./(pv.Nt+Nt);
            Mm(isnan(Mm)) = 0;
            pv.Eb = pv.Gb - Mb(:,:,ones(pv.n,1));
            if nargout > 0, E = pv.Eb; end
        end
        
        function [f,a,b] = phometry(pv,a)
            % Scatter Matrices
            Ea = scatw(pv.Fa(:,:,pv.is).*pv.Fn(:,:,pv.js),pv.Ma.*pv.Fa,pv.ks,pv.x0,pv.y0);
            Eb = scatw(pv.Na(:,:,pv.is).*pv.Nn(:,:,pv.js),pv.Na,pv.ks,pv.x0,pv.y0);
            Ec = scatw(pv.Fa(:,:,pv.ia).*pv.Nn(:,:,pv.ja),pv.Fa,pv.ka,pv.x0,pv.y0);
            wa = wnd3(pv.y0,pv.x0,pv.Fn); wb = wnd3(pv.y0,pv.x0,pv.Nn);
            S = [Eb wb; wb' 0]\[Ec wa]'; T = S(1:pv.n,:);
            Et = T'*Eb*T/2-Ec*T;
            E = Ea+Et+Et'; % error matrix with symmetry
            
            if nargin > 1, f = a'*E*a; return; end
            
            % compute the smallest generalized Eigen vector a of D
            [P,D] = eig(E);
            [f,k] = min(diag(D));
            a = P(:,k);
            % opt.A.v0 = pv.a;
            % [a,f,flag]=eigs(F,1,'sm',opt.A);
            %             if flag ~= 0
            %                 [a,f]=eig(E);
            %                 [f,i]=min(diag(f));
            %                 a = a(:,i);
            %                 fprintf('!');
            %             end
            
            if wa'*a < 0, a=-a; end % invert the negative direction
            
            % simple treatment of non-positive eigen vector
            idx = find(a < 0);
            if ~isempty(idx)
                a(idx)=0; a = a/norm(a);
                f = a'*E*a;
                fprintf('-');
            end
            pv.a = a; pv.b = -T*a; pv.f = f;
            if nargout > 2, b = pv.b; end
        end
        
        function [t,dt] = geometry(pv)
            pv.Gz = pv.X.*pv.Gy-pv.Y.*pv.Gx;
            Gx = pv.Gx./pv.Na; Gx((isnan(Gx))) = 0;
            Gy = pv.Gy./pv.Na; Gy((isnan(Gy))) = 0;
            Gz = pv.Gz./pv.Na; Gz((isnan(Gz))) = 0;
            
            Nx = pv.Gx./pv.NT; Nx((isnan(Nx))) = 0;
            Ny = pv.Gy./pv.NT; Ny((isnan(Ny))) = 0;
            Nz = pv.Gz./pv.NT; Nz((isnan(Nz))) = 0;
            %  symmetric components of Hessian
            Exx = scatw(Nx(:,:,pv.is).*pv.Gx(:,:,pv.js),Gx.*pv.Gx,pv.ks,pv.x0,pv.y0);
            Eyy = scatw(Ny(:,:,pv.is).*pv.Gy(:,:,pv.js),Gy.*pv.Gy,pv.ks,pv.x0,pv.y0);
            Ett = scatw(Nz(:,:,pv.is).*pv.Gz(:,:,pv.js),Gz.*pv.Gz,pv.ks,pv.x0,pv.y0);
            % asymmetric components of Hessian
            Exy = scatw(Nx(:,:,pv.ia).*pv.Gy(:,:,pv.ja),Gx.*pv.Gy,pv.ka,pv.x0,pv.y0);
            Ext = scatw(Nx(:,:,pv.ia).*pv.Gz(:,:,pv.ja),Gx.*pv.Gz,pv.ka,pv.x0,pv.y0);
            Eyt = scatw(Ny(:,:,pv.ia).*pv.Gz(:,:,pv.ja),Gy.*pv.Gz,pv.ka,pv.x0,pv.y0);
            % gradient components
            Gx = scatw(Nx(:,:,pv.ia).*pv.Ga(:,:,pv.ja),Gx.*pv.Ga,pv.ka,pv.x0,pv.y0);
            Gy = scatw(Ny(:,:,pv.ia).*pv.Ga(:,:,pv.ja),Gy.*pv.Ga,pv.ka,pv.x0,pv.y0);
            Gz = scatw(Nz(:,:,pv.ia).*pv.Ga(:,:,pv.ja),Gz.*pv.Ga,pv.ka,pv.x0,pv.y0);
            
            H = [Exx Exy Ext; Exy' Eyy Eyt; Ext' Eyt' Ett];
            g = sum([Gx; Gy; Gz],2);        % gradient
            
            B = pv.H;
            z = zeros(1,pv.n);
            A = [pv.sw' z z; z pv.sw' z; z z pv.sw'];
            b = -pv.sw'*B; t0 = zeros(3*pv.n,1);
            
            options = optimset('Display','off');
            [dt,fval,exitflag] = quadprog(H,g,[],[],A,b,[],[],t0,options);
            dt = reshape(dt,pv.n,3);
            
            f = pv.f;       % current squared error
            pv.H = B+dt;
            pv.proj
            pv.phometry;
            if f < pv.f,    % check smaller squared error
                pv.H = B;
                fprintf('<');
            end
            t = pv.H;
        end
        
        function grad_dr(pv)
            pv.Fr=[]; pv.Frx=[]; pv.Fry=[];
            pv.Nr=[]; pv.Nrx=[]; pv.Nry=[];
            for k=1:pv.n
                [pv.Fr(:,:,k),pv.Nr(:,:,k),pv.Frx(:,:,k),pv.Nrx(:,:,k),...
                    pv.Fry(:,:,k),pv.Nry(:,:,k)] = pv.sv(k).grad_r;
            end
            pv.Gr=pv.a(pv.Z).*pv.Fr+pv.b(pv.Z).*pv.Nr;
            pv.Grx=pv.a(pv.Z).*pv.Frx+pv.b(pv.Z).*pv.Nrx;
            pv.Gry=pv.a(pv.Z).*pv.Fry+pv.b(pv.Z).*pv.Nry;
            pv.Nt = sum(pv.Nr,3);
            pv.Ntx = sum(pv.Nrx,3);
            pv.Nty = sum(pv.Nry,3);
        end
        
        function [r,k] = ggnElevate(pv,r)
            if nargin > 1, pv.r=r; end
            r0 = pv.r; t0 = 0; pv.proj;
            p0 = reallog(pv.corelate+PatchViews.eps_p);
            [V,d,b,e] = gn(pv);
            [t,f,exitflag,output] = fminunc(@(t)mvOpt(t,pv),0,pv.opt.G);
            k = output.iterations;
            r = st(t-t0,V,d,b,r0); pv.proj;
            
            function [p,g]=mvOpt(t,pv)
                pv.r = st(t-t0,V,d,b,r0); 
                pv.proj;
                q = pv.corelate+PatchViews.eps_p;
                p = reallog(q);
                if nargout > 1
                    p0 = p;
                    for k = 1:300
                        [V,d,b,e,j] = gn(pv);
                        et = e'*e;
                        jt = 1e5*e'*j*e;
                        if jt > 0 && et > 0, t1 = et/jt/1e5; else break; end
                        p1 = mvOpt(t1,pv);
                        if p1 < p0,
                            pv.r = st(t1-t0,V,d,b,r0);
                            p0 = p1; r0 = pv.r; t0 = t1;
                        else
                            t0 = t; p = p0; break;
                        end
                    end
                    %                     t0 = t; r0 = pv.r;
%                    g = -1e5*e'*e/q;
                    g = gradest(@(t)mvOpt(t,pv),t,-inf,inf,pv.opt.G);
                end
            end
            
            function x = st(t,V,d,b,x0)
                dx = (eye(1)-V*diag(exp(-1e5*d*t))*V')*b;
                x = x0+dx;
            end
            
            function [V,d,b,e,j]=gn(pv)
                pv.jacobian;
                
                db = pv.grad_beta;
                [dp,p] = pv.grad_snr;
                dz(1) = (dp(1)*p(2)-dp(2)*p(1))/(p(1)+p(2))^2/2;
                dz(2:3) = dp(3:4);
                z = [p(1)/(p(1)+p(2))/2 p(3:4)]';
                z = z + realmin*sign(z);
                e = 2*dz*db;
                j = dz.^2*(db./z);
                [V,d,b]=gnd(j,e);
            end
        end
        
        function [r,k] = gnElevate(pv,r)
            if nargin > 1, pv.r=r; end
            pv.proj;
            pv.corelate;
            f = pv.p;
            fprintf('%d %f %f\n',0, log(f), pv.r-RasterView.radiusMoon);
            for k = 1:200
                db = pv.grad_beta;
                db(3) = db(3)-db(2);
                [dp,p] = pv.grad_snr(1e-5);
                %                [dp,p] = pv.grad_snr;
                %                [dp1,p1] = pv.grad_snr;
                dz(1) = (dp(1)*p(2)-dp(2)*p(1))/(p(1)+p(2))^2/2;
                dz(2) = dp(3)+dp(4);
                dz(3) = dp(3);
                z = [p(2)/(p(1)+p(2))/2 p(3)+p(4) p(3)]';
                res = 2*dz*db;
                jac = dz.^2*(db.*z);
                if jac ~= 0,
                    dr = -pinv(db.*dz')*(z.*(pinv(dz)*res)); %-res/jac;
                    pv.r = pv.r+dr;
                else
                    break;
                end
                pv.proj;
                pv.corelate;
                if log(pv.p)-log(f) < 1e-6, f = pv.p; else pv.r = pv.r-dr;
                    break;
                end
                fprintf('%d %f %f %f\n',k, log(f), pv.r-RasterView.radiusMoon, dr);
            end
            r = pv.r;
        end
        
        function g = grad_p(pv)
            a = pv.ne; b = pv.ns;
            x = pv.se/(pv.se+pv.ss); y = 1-x;
            %             a = pv.ne; b = pv.ns; c = a+b; f = pv.p;
            %             pa = psi(a); pb = psi(b); pc = psi(c);
            %             ga = gamma(a); gb = gamma(b); gc = gamma(c);
            %             la = gammaln(a); lb = gammaln(b); lc = gammaln(c);
            %             Fx = hypergeom([a a 1-b],[a+1 a+1],x);
            %             Fy = hypergeom([b b 1-a],[b+1 b+1],y);
            %             Dx = exp(la+lb-lc+a*log(x)+log(Fx)); % exp(la+lc-lb+a*log(x)+log(Fx));
            %             dg(2) = (log(x)-pa+pc)*f-Dx;
            %             Dy = exp(la+lb-lc+b*log(y)+log(Fy)); % exp(lb+lc-la+b*log(y)+log(Fy));
            %             dg(3) = Dy-(log(y)-pb+pc)*f;
            dg(1) = exp((b-1)*log(y)+(a-1)*log(x)-betaln(a,b));
            dg(2) = gradest(@(a)betainc(x,a,b),a,0,inf,pv.opt.T);
            dg(3) = gradest(@(b)betainc(x,a,b),b,0,inf,pv.opt.T);
            
            de = pv.grad_ne; ds = pv.grad_ns;
            dy = pv.ss*pv.grad_se; dx = pv.se*pv.grad_ss;
            g = dg(1)*(dy-dx)/(pv.se+pv.ss)^2+pv.rof*(dg(2)*de+dg(3)*ds);
        end
        
        function g = grad_beta(pv)
            a = pv.ne; b = pv.ns;
            x = pv.se/(pv.se+pv.ss); y = 1-x;
            g(1,1) = exp((b-1)*log(y)+(a-1)*log(x)-betaln(a,b));
            g(2,1) = gradest(@(a)betainc(x,a,b),a,0,inf,pv.opt.T);
            g(3,1) = gradest(@(b)betainc(x,a,b),b,0,inf,pv.opt.T);
        end
        
        function [dp,p0] = grad_snr(pv,dr)
            p0 = [pv.se pv.ss pv.ne pv.ns pv.f];
            if nargin > 1
                pv.r = pv.r+dr; pv.proj; pv.corelate;
                p1 = [pv.se pv.ss pv.ne pv.ns pv.f];
                dp = (p1-p0)/dr;
            else
                pv.jacobian;
                ns = pv.dNsdQ;
                dp = [pv.dSedQ pv.dSsdQ pv.dNtdQ-ns ns];
            end
        end
        
        function g = grad_se(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_se(c,pv.Mm,pv.Ma(:,:,k),pv.Gb(:,:,k));
            end
        end
        
        function g = dSdQ(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_se(c,pv.Mm,pv.Ma(:,:,k),pv.Gb(:,:,k));
            end
        end
        
        function g = dEdQ(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_se(c,pv.Mm,pv.Ma(:,:,k),pv.Gb(:,:,k));
            end
        end
        
        function g = dAdQ(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_se(c,pv.Mm,pv.Ma(:,:,k),pv.Gb(:,:,k));
            end
        end
        
        function g = dBdQ(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_se(c,pv.Mm,pv.Ma(:,:,k),pv.Gb(:,:,k));
            end
        end
        
        function g = grad_ss(pv)
            g = []; c = pv.y0*pv.x0';
            Nx = sum(pv.Nx,3); Ny = sum(pv.Ny,3);
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ss(c,pv.Gb(:,:,k),...
                    pv.Mm,pv.Mx,pv.My,pv.Nt,Nx,Ny);
            end
        end
        
        function g = grad_ms(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ms(c,pv.Nt,pv.Gb(:,:,k),pv.Mm);
            end
        end
        
        function g = grad_mx(pv)
            g = []; c = pv.y0*pv.x0';
            Nx = sum(pv.Nx,3);
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_mx(c,pv.Nt,Nx,...
                    pv.Gb(:,:,k),pv.Mm);
            end
        end
        
        function g = grad_my(pv)
            g = []; c = pv.y0*pv.x0';
            Ny = sum(pv.Ny,3);
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_my(c,pv.Nt,Ny,...
                    pv.Gb(:,:,k),pv.Mm);
            end
        end
        
        function g = grad_gs(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_gs(c,pv.Gb(:,:,k));
            end
        end
        
        function g = grad_gx(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_gx(c,pv.Gb(:,:,k));
            end
        end
        
        function g = grad_gy(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_gy(c,pv.Gb(:,:,k));
            end
        end
        
        function g = grad_ws(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ws(c);
            end
        end
        
        function g = grad_wx(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_wx(c);
            end
        end
        
        function g = grad_wy(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_wy(c);
            end
        end
        
        function g = grad_nt(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_nt(c);
            end
        end
        
        function g = dNtdQ(pv)
            g = pv.rof*sum(wnd3(pv.y0,pv.x0,pv.Nrt));
        end
        
        function g = dNsdQ(pv)
            g = pv.rof*sum(wnd3(pv.y0,pv.x0,pv.Nrs));
        end
        
        function g = dSedQ(pv)
            D2 = pv.Ga.^2./pv.Na; D2(isnan(D2)) = 0;
            D2r = (2*pv.Ga.*pv.Gr-D2.*pv.Nr)./pv.Na; D2r(isnan(D2r)) = 0;
            g = sum(wnd3(pv.y0,pv.x0,D2r));
            
            M2 = pv.Gt.^2./pv.Nt; M2(isnan(M2)) = 0;
            M2r = (2*pv.Gt.*pv.Grt-M2.*pv.Nrt)./pv.Nt; M2r(isnan(M2r)) = 0;
            g = g-wnd3(pv.y0,pv.x0,M2r);
        end
        
        function g = dSsdQ(pv)
            gx = pv.dSxdQ; gy = pv.dSydQ;
            g = gx+gy;
        end
        
        function g = dSxdQ(pv)
            nGx = pv.Gtx-pv.Mm.*pv.Ntx;
            Sx = nGx.^2./pv.Nt; Sx(isnan(Sx)) = 0;
            nGrx = pv.Grtx-pv.Mr.*pv.Ntx-pv.Mm.*pv.Nrtx;
            Sr = (2*nGrx.*nGx-pv.Nrt.*Sx)./pv.Nt; Sr(isnan(Sr)) = 0;
            g = wnd3(pv.y0,pv.x0,Sr);
        end
        
        function g = dSydQ(pv)
            nGy = pv.Gty-pv.Mm.*pv.Nty;
            Sy = nGy.^2./pv.Nt; Sy(isnan(Sy)) = 0;
            nGry = pv.Grty-pv.Mr.*pv.Nty-pv.Mm.*pv.Nrty;
            Sr = (2*nGry.*nGy-pv.Nrt.*Sy)./pv.Nt; Sr(isnan(Sr)) = 0;
            g = wnd3(pv.y0,pv.x0,Sr);
        end
        
        function g = grad_ns(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ns(c,pv.Ns,pv.Nt);
            end
        end
        
        function g = grad_ne(pv)
            g = pv.grad_nt - pv.grad_ns;
        end
        
        function c = robust(pv)
            E = pv.residual;
            if pv.d(1) == 0
                pv.d(1) = 1/std(E(:))/sqrt(5);
                %             else
                %                 pv.d(1) = pv.d(1)/2;
            end
            pv.opt.W = optimset(pv.opt.W,'OutputFcn',@outres);
            %            [c,f,exitflag,output] = fmincon(@(c)mvOpt(pv,c),pv.d,[],[],[],[],[0 4],[inf inf],[],pv.opt.W);
            [c,f,exitflag,output] = fminunc(@(c)mvOptBnd(pv,c),pv.d(1),pv.opt.W);
            %             E = pv.residual; lb = -max(abs(E(:)));
            %             [c,f,exitflag,output] = fminbnd(@(c)mvOpt(c,pv),lb,0,pv.opt.W);
            if ~isequal(pv.opt.W.Display,'off'),
                output
                exitflag
                pv.disp
            end
            
            function stop = outres(c,optimvalues,state)
                stop = false;
                switch state
                    case 'iter'
                        pv.residual;
                end
            end
            %             function p = mvOpt(c,pv)
            %                 E = pv.residual;
            %                 pv.W = ones(size(E));
            %                 pv.W(find(abs(E)>-c))=0;
            %                 for k=1:pv.n, pv.sv(k).W = pv.W(:,:,k); end
            %                 pv.proj;
            %                 p=reallog(pv.corelate+PatchViews.eps_p);
            %             end
        end
        
        function e = robusti(pv)
            sz = size(pv.Eb);
            n = prod(sz(1:2))*(sz(3)-1);
            P = [];
            for k = 1:1000
                pv.residual;
                pv.Eb(~pv.W) = 0;
                [e,i]=max(abs(pv.Eb(:)));
                pv.W(i) = 0; pv.proj;
                p = reallog(pv.corelate+PatchViews.eps_p);
                P = [P p];
                fprintf('.');
            end
            figure, plot(P)
        end
        
        function p = mvOpt(pv,c)
            %            E = pv.residual;
            pv.W = exp(-(abs(pv.Eb)*c(1)).^c(2));
            pv.proj;
            p=reallog(pv.corelate+PatchViews.eps_p);
        end
        
        function p = mvOptBnd(pv,c)
            pv.W = exp(-(abs(pv.Eb)*c).^pv.d(2));
            pv.proj;
            p=reallog(pv.corelate+PatchViews.eps_p);
        end
        
        function [p,a,b,f] = corelate(pv)
            [f,a,b]=pv.phometry;    % photometric estimation
            pv.gradient;            % gradient computation
            
            % Confidence Value
            s = wnd3(pv.y0,pv.x0,pv.G2);                % signal
            pv.sw = wnd3(pv.y0,pv.x0,pv.Na); sw = sum(pv.sw);   % individual dofs
            sv = wnd3(pv.y0,pv.x0,pv.Ns);                % signal dofs
            pv.nt = pv.rof*sw;
            pv.ns = pv.rof*sv;
            pv.ne = pv.nt-pv.ns-2;
            if any([s f pv.ns pv.ne] < 0),
                fprintf('All values and dofs of signal and error should be non-negative\n');
                fprintf('signal: %f, dof: %f\n',s,pv.ns);
                fprintf(' error: %f, dof: %f\n',f,pv.ne);
            end
            p = fpval(s,pv.m*f,pv.ns,pv.ne);
            pv.p = p; pv.se = f; pv.ss = s;
        end
        
        function [G2,Gx,Gy] = gradient(pv)
            % Gx, Gy: gradients with constant weight
            % G2: weighted gradient
            pv.Ga = pv.a(pv.Z).*pv.Fa+pv.b(pv.Z).*pv.Na;
            pv.Gx = pv.a(pv.Z).*pv.Fx+pv.b(pv.Z).*pv.Nx;
            pv.Gy = pv.a(pv.Z).*pv.Fy+pv.b(pv.Z).*pv.Ny;
            pv.Gb = pv.a(pv.Z).*pv.Fb+pv.b(pv.Z);
            
            pv.Gt = sum(pv.Ga,3);
            pv.Gtx = sum(pv.Gx,3); pv.Gty = sum(pv.Gy,3);
            pv.Ntx = sum(pv.Nx,3); pv.Nty = sum(pv.Ny,3);
            pv.Mm = pv.Gt./pv.Nt; pv.Mm(isnan(pv.Mm)) = 0;
            
            pv.Mx = sum(pv.Gx,3)-pv.Mm.*pv.Gtx;
            pv.My = sum(pv.Gy,3)-pv.Mm.*pv.Gty;
            
            pv.G2 = (pv.Mx.^2+pv.My.^2)./pv.Nt; pv.G2(isnan(pv.G2)) = 0;
            
            if nargout > 0, Gx = pv.Gx; end
            if nargout > 1, Gy = pv.Gy; end
            if nargout > 2, G2 = pv.G2; end
        end
        
        function jacobian(pv)
            % total and normalized weights
            pv.Nr=[]; pv.Nrx=[]; pv.Nry=[];
            pv.Fr=[]; pv.Frx=[]; pv.Fry=[];
            for k=1:pv.n
                [pv.Fr(:,:,k),pv.Nr(:,:,k),pv.Frx(:,:,k),pv.Nrx(:,:,k),...
                    pv.Fry(:,:,k),pv.Nry(:,:,k)] = pv.sv(k).grad_r;
            end
            
            pv.Gr = pv.a(pv.Z).*pv.Fr+pv.b(pv.Z).*pv.Nr;
            pv.Grx = pv.a(pv.Z).*pv.Frx+pv.b(pv.Z).*pv.Nrx;
            pv.Gry = pv.a(pv.Z).*pv.Fry+pv.b(pv.Z).*pv.Nry;
            
            pv.Grt = sum(pv.Gr,3);   pv.Nrt = sum(pv.Nr,3);
            pv.Grtx = sum(pv.Grx,3); pv.Nrtx = sum(pv.Nrx,3);
            pv.Grty = sum(pv.Gry,3); pv.Nrty = sum(pv.Nry,3);
            
            pv.Nrs = (2*sum(pv.Nr.*pv.Na,3)-pv.Nrt.*pv.Ns)./pv.Nt;
            pv.Nrs(isnan(pv.Nrs)) = 0;
            
            pv.Mr = (pv.Grt-pv.Mm.*pv.Nrt)./pv.Nt;
            pv.Mr(isnan(pv.Mr)) = 0;
        end
        
        function slowate(pv)
            pv.gradient;            % gradient computation
            
            Ga = sum(pv.Ga,3);
            Gx = sum(pv.Gx,3); Gy = sum(pv.Gy,3);
            Nx = sum(pv.Nx,3); Ny = sum(pv.Ny,3);
            pv.ms = wnd3(pv.y0,pv.x0,pv.Mm);
            pv.mx = wnd3(pv.y0,pv.x0,pv.Mx);
            pv.my = wnd3(pv.y0,pv.x0,pv.My);
            pv.gs = wnd3(pv.y0,pv.x0,Ga);
            pv.gx = wnd3(pv.y0,pv.x0,Gx);
            pv.gy = wnd3(pv.y0,pv.x0,Gy);
            pv.ws = wnd3(pv.y0,pv.x0,pv.Nt);
            pv.wx = wnd3(pv.y0,pv.x0,Nx);
            pv.wy = wnd3(pv.y0,pv.x0,Ny);
            
            pv.se = pv.phometry(pv.a);    % photometric estimation
            pv.ss = wnd3(pv.y0,pv.x0,pv.G2);                % signal
            
            % Confidence Value
            sv = wnd3(pv.y0,pv.x0,pv.Ns);                % signal dofs
            pv.sw = wnd3(pv.y0,pv.x0,pv.Na); sw = sum(pv.sw);   % individual dofs
            pv.nt = pv.rof*sw;
            pv.ns = pv.rof*sv;
            pv.ne = pv.nt-pv.ns-2;
            pv.p = fpval(pv.ss,pv.se,pv.ns,pv.ne);
        end
        
        function set.q(obj,q) % q property set function
            obj.q = q;
        end % set.q
        
        function q=get.q(obj) % q property set function
            q = obj.q;
        end % get.q
        
        function set.e(obj,e) % e property set function
            obj.e = e;
        end % set.e
        
        function e=get.e(obj) % e property set function
            e = obj.e;
        end % get.e
        
        function set.p(obj,p) % p property set function
            obj.p = p;
        end % set.p
        
        function p=get.p(obj) % p property set function
            p = obj.p;
        end % get.p
        
        function set.ne(obj,ne) % ne property set function
            obj.ne = ne;
        end % set.ne
        
        function ne=get.ne(obj) % ne property set function
            ne = obj.ne;
        end % get.ne
        
        function set.ns(obj,ns) % ns property set function
            obj.ns = ns;
        end % set.ns
        
        function ns=get.ns(obj) % ns property set function
            ns = obj.ns;
        end % get.ns
        
        function m=addRatio(pv)
            pv.m = pv.m*PatchViews.ratioMulti;
            if nargout > 0, m = pv.m; end
        end
        
        function set.u(obj,u) % u property set function
            obj.u = u;
        end % set.u
        
        function u=get.u(obj) % u property set function
            u = obj.u;
        end % get.u
        
    end % methods
    
end % classdef

function [f0 f1 f2]=fltGaussian(r,s,c)
% r: radius, s: sigma
if nargin < 3, c = 0; end
x=[-r-0.5:r+0.5]'+c;
fx = normpdf(x,0,s); fx(1) = 0; fx(end) = 0;
Fx = normcdf(x,0,s); Fx(1) = 0; Fx(end) = 1;
f0 = diff(Fx); f1 = diff(fx);
f2 = diff(-x.*fx)/s^2;
end

function [w0 w1 w2]=wndGaussian(r,s)
% r: radius, s: sigma
[w0 w1 w2]=fltGaussian(r,s);
w1 = -w1*s^2;
w2 = s^2*(s^2*w2+w0);
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
if optimvalues.fval < PatchViews.eps_log2p,
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

function [V,d,b] = gnd(A,b) % Gauss-Newton Decomposition
[V,D]=eig(A);
d = diag(D);
if nargin > 1, b = -lscov(A,b); end
end
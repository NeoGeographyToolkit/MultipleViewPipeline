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
        c = [40 40]';   % extended window
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
        Fa, Fb, Fs, Fx, Fy, 
        Ga, Gb, Gs, Gx, Gy, Gt, G2      % albedo gradients
                Ms, Mx, My
        Es, Eb  % albedos
        Wb, Ws, Wx, Wy
        Wt, Wn, Wr      % total and normalized weight
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
                addlistener(pv,'c','PostSet',@obj.PropEvents);
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
            pv.c = pv.w(1:2);
            pv.dw = pv.w(1:2);
            pv.W = ones(size(pv.X));
            pv.r = RasterView.radiusMoon;
            
            pv.opt.R = optimset('disp','iter','Largescale','off');
            pv.opt.S = optimset(pv.opt.R,'MaxIter',1,'Algorithm','interior-point');
            pv.opt.W = optimset(pv.opt.R,'MaxIter',0,'TolX',1e-25);
            pv.opt.W = optimset(pv.opt.W,'GradObj','off');
            pv.opt.T = optimset('Largescale','on','FinDiffType','central');
            pv.opt.T = optimset(pv.opt.T,'MaxIter',0,'DerivativeCheck','on');
            pv.opt.T = optimset(pv.opt.T,'GradObj','on','OutputFcn',@outfun);
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
            r = pv.elevate;
            h = r-RasterView.radiusMoon;
            fprintf('The elevation is %f\n', h);
            
            pv.opt.Q = optimset(pv.opt.R,'OutputFcn',@rotfun);
            pv.opt.R = optimset(pv.opt.R,'disp','off','OutputFcn',@radfun);
            pv.opt.S = optimset(pv.opt.S,'disp','off');
            [q,f,exitflag,output] = fminunc(@(q)mvOpt(q,pv),pv.q,pv.opt.Q);
            pv.q=q; pv.proj;
            if ~isequal(pv.opt.Q.Display,'off'),
                output
                exitflag
                pv.disp
            end
            
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
                        t = pv.scale.^2;
                        fprintf('The scale is (%f,%f)\n', t(1),t(2));
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
            
            if ~isempty(pv.Fs)
                pv.residual;
                A = num2cell(pv.Fa,[1 2]);
                A = reshape(A,[2 2]);
                A = cell2mat(A);
                
                h = figure;
                subplot(2,3,1), imshow(imadjust(A)), axis equal
                title('Before Photometric Correction','FontSize',14)
                
                C = num2cell(pv.Ga,[1 2]);
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
                
                subplot(2,3,3), imagesc(pv.Ms), axis off equal
                title('Estimated Albedo','FontSize',14)
                
                subplot(2,3,4), imagesc(E), axis off equal
                title('Standard Error','FontSize',14)
            end
        end
        
        function PropEvents(pv,src,evt)
            switch src.Name
                case 'c'
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
                        if ~isequal(pv.sv(k).W(pv.sv(k).is,pv.sv(k).js),W(:,:,k)),
                            pv.sv(k).W(pv.sv(k).is,pv.sv(k).js) = pv.W(:,:,k);
                        end
                    end
            end
        end
        
        function initWindows(pv)
            w = ceil([pv.t(1:2)*PatchViews.ratioScale; pv.s*SingleView.ratioSmoth]);
            if ~isequal(pv.w,w),
                if any(pv.c < pv.w(1:2)),
                    error('pv.c should be bigger than pv.w');
                end
                pv.w = w;
                [pv.X, pv.Y pv.Z]=meshgrid(-w(1):w(1),-w(2):w(2),1:pv.n);
                pv.W = ones(size(pv.X));
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
            pv.Fs=[]; pv.Fx=[]; pv.Fy=[]; pv.Fb=[];
            pv.Ws=[]; pv.Wx=[]; pv.Wy=[]; pv.Wb=[];
            if isequal(pv.c,pv.w(1:2))
                for k=1:pv.n, pv.sv(k).smooth; end
            end
            for k=1:pv.n
                [pv.Fs(:,:,k),pv.Ws(:,:,k),pv.Fx(:,:,k),pv.Wx(:,:,k),...
                    pv.Fy(:,:,k),pv.Wy(:,:,k),pv.Fb(:,:,k),pv.Wb(:,:,k)] = pv.sv(k).crop;
            end
            
            % total and normalized weights
            pv.Wt = sum(pv.Ws,3);
            pv.Wn = pv.Ws./repmat(pv.Wt,[1 1 pv.n]);
            pv.Wn(isnan(pv.Wn)) = 0;
            pv.Wr = sum(pv.Wn.*pv.Ws,3);
       
            if nargout > 0, I = pv.Fs; end
            if nargout > 1, W = pv.Ws; end
        end
        
        function E = residual(pv)
            pv.Gb = pv.a(pv.Z).*pv.Fb+pv.b(pv.Z);
            pv.Ga = pv.Gs./pv.Ws; pv.Ga((isnan(pv.Ga))) = 0;
            pv.Ms = sum(pv.Gs,3)./pv.Wt; pv.Ms((isnan(pv.Ms))) = 0;
            pv.Es = pv.Gb - pv.Ms(:,:,ones(pv.n,1));
            Wt = sum(pv.W.*pv.Wb,3); Gb = sum(pv.Gb,3);
            Mb = Gb./Wt; Mb(isnan(Mb)) = 0;
            Ma = (pv.Wt.*pv.Ms+Wt.*Mb)./(pv.Wt+Wt);
            Ma(isnan(Ma)) = 0;
            pv.Eb = pv.Gb - Mb(:,:,ones(pv.n,1));
            if nargout > 0, E = pv.Eb; end
        end
        
        function [f,a,b] = phometry(pv,a)
            % Scatter Matrices
            pv.Fa = pv.Fs./pv.Ws; pv.Fa((isnan(pv.Fa))) = 0;
            In = pv.Fs./repmat(pv.Wt,[1 1 pv.n]); In((isnan(In))) = 0;
            Ea = scatw(pv.Fs(:,:,pv.is).*In(:,:,pv.js),pv.Fa.*pv.Fs,pv.ks,pv.x0,pv.y0);
            Eb = scatw(pv.Ws(:,:,pv.is).*pv.Wn(:,:,pv.js),pv.Ws,pv.ks,pv.x0,pv.y0);
            Ec = scatw(pv.Fs(:,:,pv.ia).*pv.Wn(:,:,pv.ja),pv.Fs,pv.ka,pv.x0,pv.y0);
            wa = wnd3(pv.y0,pv.x0,In); wb = wnd3(pv.y0,pv.x0,pv.Wn);
            S = [Eb wb; wb' 0]\[Ec; wa'];
            T = S(1:pv.n,:);
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
            
            if sum(a) < 0, a=-a; end % invert the negative direction
            
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
            pv.Gt = pv.X.*pv.Gy-pv.Y.*pv.Gx;
            Gx = pv.Gx./pv.Ws; Gx((isnan(Gx))) = 0;
            Gy = pv.Gy./pv.Ws; Gy((isnan(Gy))) = 0;
            Gt = pv.Gt./pv.Ws; Gt((isnan(Gt))) = 0;
            WT = repmat(pv.Wt,[1 1 pv.n]);
            Nx = pv.Gx./WT; Nx((isnan(Nx))) = 0;
            Ny = pv.Gy./WT; Ny((isnan(Ny))) = 0;
            Nt = pv.Gt./WT; Nt((isnan(Nt))) = 0;
            %  symmetric components of Hessian
            Exx = scatw(Nx(:,:,pv.is).*pv.Gx(:,:,pv.js),Gx.*pv.Gx,pv.ks,pv.x0,pv.y0);
            Eyy = scatw(Ny(:,:,pv.is).*pv.Gy(:,:,pv.js),Gy.*pv.Gy,pv.ks,pv.x0,pv.y0);
            Ett = scatw(Nt(:,:,pv.is).*pv.Gt(:,:,pv.js),Gt.*pv.Gt,pv.ks,pv.x0,pv.y0);
            % asymmetric components of Hessian
            Exy = scatw(Nx(:,:,pv.ia).*pv.Gy(:,:,pv.ja),Gx.*pv.Gy,pv.ka,pv.x0,pv.y0);
            Ext = scatw(Nx(:,:,pv.ia).*pv.Gt(:,:,pv.ja),Gx.*pv.Gt,pv.ka,pv.x0,pv.y0);
            Eyt = scatw(Ny(:,:,pv.ia).*pv.Gt(:,:,pv.ja),Gy.*pv.Gt,pv.ka,pv.x0,pv.y0);
            % gradient components
            Gx = scatw(Nx(:,:,pv.ia).*pv.Gs(:,:,pv.ja),Gx.*pv.Gs,pv.ka,pv.x0,pv.y0);
            Gy = scatw(Ny(:,:,pv.ia).*pv.Gs(:,:,pv.ja),Gy.*pv.Gs,pv.ka,pv.x0,pv.y0);
            Gt = scatw(Nt(:,:,pv.ia).*pv.Gs(:,:,pv.ja),Gt.*pv.Gs,pv.ka,pv.x0,pv.y0);
            
            H = [Exx Exy Ext; Exy' Eyy Eyt; Ext' Eyt' Ett];
            g = sum([Gx; Gy; Gt],2);        % gradient
            
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
        
        function g = grad_se(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_se(c,pv.Ms,pv.Ga(:,:,k),pv.Gb(:,:,k));
            end
        end
        
        function g = grad_ss(pv)
            g = []; c = pv.y0*pv.x0';
            Wx = sum(pv.Wx,3); Wy = sum(pv.Wy,3);
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ss(c,pv.Gb(:,:,k),...
                    pv.Ms,pv.Mx,pv.My,pv.Wt,Wx,Wy);
            end
        end
        
        function g = grad_ms(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ms(c,pv.Wt,pv.Gb(:,:,k),pv.Ms);
            end
        end
        
        function g = grad_mx(pv)
            g = []; c = pv.y0*pv.x0';
            Wx = sum(pv.Wx,3);
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_mx(c,pv.Wt,Wx,...
                    pv.Gb(:,:,k),pv.Ms);
            end
        end
        
        function g = grad_my(pv)
            g = []; c = pv.y0*pv.x0';
            Wy = sum(pv.Wy,3);
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_my(c,pv.Wt,Wy,...
                    pv.Gb(:,:,k),pv.Ms);
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
        
        function g = grad_ns(pv)
            g = []; c = pv.y0*pv.x0';
            for k = 1:pv.n
                g(:,:,k) = pv.sv(k).grad_ns(c,pv.Wr,pv.Wt);
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
            pv.sw = wnd3(pv.y0,pv.x0,pv.Ws); sw = sum(pv.sw);   % individual dofs
            sv = wnd3(pv.y0,pv.x0,pv.Wr);                % signal dofs
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
        
        function slowate(pv)
            pv.gradient;            % gradient computation
            
            Gs = sum(pv.Gs,3); 
            Gx = sum(pv.Gx,3); Gy = sum(pv.Gy,3);
            Wx = sum(pv.Wx,3); Wy = sum(pv.Wy,3);
            pv.ms = wnd3(pv.y0,pv.x0,pv.Ms);
            pv.mx = wnd3(pv.y0,pv.x0,pv.Mx);
            pv.my = wnd3(pv.y0,pv.x0,pv.My);
            pv.gs = wnd3(pv.y0,pv.x0,Gs);
            pv.gx = wnd3(pv.y0,pv.x0,Gx);
            pv.gy = wnd3(pv.y0,pv.x0,Gy);
            pv.ws = wnd3(pv.y0,pv.x0,pv.Wt);
            pv.wx = wnd3(pv.y0,pv.x0,Wx);
            pv.wy = wnd3(pv.y0,pv.x0,Wy);

            pv.se = pv.phometry(pv.a)/2;    % photometric estimation
            pv.ss = wnd3(pv.y0,pv.x0,pv.G2)/2;                % signal
            
            % Confidence Value
            sv = wnd3(pv.y0,pv.x0,pv.Wr);                % signal dofs
            pv.sw = wnd3(pv.y0,pv.x0,pv.Ws); sw = sum(pv.sw);   % individual dofs
            pv.nt = pv.rof*sw;
            pv.ns = pv.rof*sv;
            pv.ne = pv.nt-pv.ns-2;
            pv.p = fpval(pv.ss,pv.se,pv.ns,pv.ne);
        end
        
        function [G2,Gx,Gy] = gradient(pv)
            % Gx, Gy: gradients with constant weight
            % G2: weighted gradient
            pv.Gs = pv.a(pv.Z).*pv.Fs+pv.b(pv.Z).*pv.Ws;
            pv.Gx = pv.a(pv.Z).*pv.Fx+pv.b(pv.Z).*pv.Wx;
            pv.Gy = pv.a(pv.Z).*pv.Fy+pv.b(pv.Z).*pv.Wy;
            pv.Gb = pv.a(pv.Z).*pv.Fb+pv.b(pv.Z);
            
            Gt = sum(pv.Gs,3); Wx = sum(pv.Wx,3); Wy = sum(pv.Wy,3);
            pv.Ms = Gt./pv.Wt; pv.Ms(isnan(pv.Ms)) = 0;
            pv.Ga = pv.Gs./pv.Ws; pv.Ga((isnan(pv.Ga))) = 0;
            
            pv.Mx = sum(pv.Gx,3)-pv.Ms.*Wx;
            pv.My = sum(pv.Gy,3)-pv.Ms.*Wy;
            
            pv.G2 = (pv.Mx.^2+pv.My.^2)./pv.Wt; pv.G2(isnan(pv.G2)) = 0;
            
            if nargout > 0, Gx = pv.Gx; end
            if nargout > 1, Gy = pv.Gy; end
            if nargout > 2, G2 = pv.G2; end
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

function [p,p1,p2] = fbval(ss,se,ns,ne,eps_p)
if nargin<5, eps_p = realmin; end
p1 = fpval(ss,se,ns,ne,eps_p);
p2 = fpval(ns^2*se,ne^2*ss,ns,ne,eps_p);
p = abs(p1-p2);
end
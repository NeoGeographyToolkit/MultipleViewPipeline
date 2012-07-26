classdef PatchViews < handle
    % terrain patch contains handle of single view
    properties (Constant)
        ratioScale = 5;
        ratioSmoth = 4;
        eps_realmin = realmin;
        eps_p = realmin;
        eps_log2p = reallog(2*realmin);
        eps_log = reallog(realmin);
        eps_rescale = eps;
        max_iter = 100;
    end % properties (Constant)
    
    properties (SetObservable)
        n = 2;          % number of patches
        r = 0;          % radial elevation
        e = [0 0 1]     % elevation post
        q = [0 0 0 1]'; % quaternion of rotation
        s = 0.5;        % smoothing scale
        t = [10 10]';   % correlation scale
        u = 31;         % unit length of the pixel
        w
        H
        W
    end
    
    properties (Hidden)
        sv = SingleView;
        Ia, Is, Ix, Iy, Ws, Wx, Wy
        It              % gradient image w.r.t theta
        Ib              % original patch
        Wt, Wn, Wr      % total and normalized weight
        Ic, Wc          % corrected by linear reflectance
        n2, N, nc, is, js, ks, ia, ja, ka,
        x0, x1, x2, y0, y1, y2,
        Gb, Gs, Gm, Ga, Es, Eb  % albedos
        Gx, Gy, Gt, G2      % albedo gradients
        Ds              % disparity
        rof, sof   % reciprocals of dof
        ne, nr, ns, nt  % degrees of freedom
        a, b;     % reflectance coefficients
        m = 1;          % multiplicative hypothesis
        p               % p-value of correspondences
        f, df           % squared error
        X, Y, Z
        sw              % individual total dofs
        
        opt;            % optimization settings
    end
    
    methods
        function pv = PatchViews(sv,e)
            if nargin == 0, return; end
            addlistener(pv,'n','PostSet',@pv.PropEvents);
            addlistener(pv,'e','PostSet',@pv.PropEvents);
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
            pv.r = pv.r;
            pv.s = pv.s;
            pv.t = pv.t;
            
            pv.opt.R = optimset('disp','iter','Largescale','off');
            pv.opt.R = optimset(pv.opt.R,'Largescale','off');
            pv.opt.W = optimset('disp','iter','Largescale','off');
            pv.opt.W = optimset(pv.opt.W,'TolX',1e-25,'FinDiffType','central');
            pv.opt.T = optimset('Largescale','off','FinDiffType','central');
            pv.opt.T = optimset(pv.opt.T,'MaxIter',0,'DerivativeCheck','on');
            pv.opt.T = optimset(pv.opt.T,'GradObj','on');
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
                output
                exitflag
                pv.disp
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
        
        function q = translate(pv)
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
        
        function h = disp(pv)
            if ~isempty(pv.W)
                figure;
                C = num2cell(pv.W,[1 2]);
                C = reshape(C,[2 2]);
                C = cell2mat(C);
                figure, imagesc(C), axis equal, colorbar;
            end
            
            if ~isempty(pv.Ia)
                pv.residual;
                A = num2cell(pv.Ia,[1 2]);
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
                
                subplot(2,3,3), imagesc(pv.Gm), axis off equal
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
            w = ceil([pv.t(1:2)*PatchViews.ratioScale; pv.s*PatchViews.ratioSmoth]);
            if ~isequal(pv.w,w),
                pv.w = w;
                [pv.X, pv.Y pv.Z]=meshgrid(-w(1):w(1),-w(2):w(2),1:pv.n);
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
            pv.Is=[]; pv.Ix=[]; pv.Iy=[]; pv.Ib=[];
            pv.Ws=[]; pv.Wx=[]; pv.Wy=[]; W=[];
            for k=1:pv.n
                [pv.Is(:,:,k),pv.Ws(:,:,k),pv.Ix(:,:,k),pv.Wx(:,:,k),...
                    pv.Iy(:,:,k),pv.Wy(:,:,k),pv.Ib(:,:,k),W(:,:,k)] = pv.sv(k).crop;
            end
            
            pv.W = W;
            % total and normalized weights
            pv.Wt = sum(pv.Ws,3);
            pv.Wn = pv.Ws./repmat(pv.Wt,[1 1 pv.n]);
            pv.Wn(isnan(pv.Wn)) = 0;
            pv.Wr = sum(pv.Wn.*pv.Ws,3);

            
            if nargout > 0, I = pv.Is; end
            if nargout > 1, W = pv.Ws; end
        end
        
        function E = residual(pv)
            pv.Gb = pv.a(pv.Z).*pv.Ib+pv.b(pv.Z);
            pv.Ga = pv.Gs./pv.Ws; pv.Ga((isnan(pv.Ga))) = 0;
            pv.Gm = sum(pv.Gs,3)./pv.Wt; pv.Gm((isnan(pv.Gm))) = 0;
            pv.Es = pv.Ga - pv.Gm(:,:,ones(pv.n,1));
            pv.Eb = pv.Gb - pv.Gm(:,:,ones(pv.n,1));
            if nargout > 0, E = pv.Eb; end
        end
        
        function [f,a,b] = phometry(pv)
            % Scatter Matrices
            pv.Ia = pv.Is./pv.Ws; pv.Ia((isnan(pv.Ia))) = 0;
            In = pv.Is./repmat(pv.Wt,[1 1 pv.n]); In((isnan(In))) = 0;
            Ea = scatw(pv.Is(:,:,pv.is).*In(:,:,pv.js),pv.Ia.*pv.Is,pv.ks,pv.x0,pv.y0);
            Eb = scatw(pv.Ws(:,:,pv.is).*pv.Wn(:,:,pv.js),pv.Ws,pv.ks,pv.x0,pv.y0);
            Ec = scatw(pv.Is(:,:,pv.ia).*pv.Wn(:,:,pv.ja),pv.Is,pv.ka,pv.x0,pv.y0);
            wa = wnd3(pv.y0,pv.x0,In); wb = wnd3(pv.y0,pv.x0,pv.Wn);
            T = (Eb*Eb+wb*wb')\(Eb*Ec'+wb*wa');
            Et = T'*Eb*T/2-Ec*T;
            E = Ea+Et+Et'; % error matrix with symmetry
            
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
        
        function w = testGradients(pv)
            pv.test_grad_ns;
            pv.test_grad_nt;
        end
        
        function w = test_grad_nt(pv)
            W = pv.W; t = pv.t;
            pv.t = [1 1]'; pv.proj; pv.disp;
            w = pv.W(:); [f,g] = mvOpt(w,pv);
            g = reshape(g,size(pv.Ws));
            figure, imagesc(g(:,:,1)), colorbar;
            [w,f,exitflag,output] = fminunc(@(w)mvOpt(w,pv),w,pv.opt.T);
            pv.t = t; pv.proj; pv.W = W;
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.nt;
                if nargout > 1, g = pv.rof*pv.grad_nt; end
            end
        end
        
        function w = test_grad_ns(pv)
            W = pv.W; t = pv.t;
            pv.t = [1 1]'; pv.proj; pv.disp;
            w = pv.W(:); [f,g] = mvOpt(w,pv);
            g = reshape(g,size(pv.Ws));
            figure, imagesc(g(:,:,1)), colorbar;
            [w,f,exitflag,output] = fminunc(@(w)mvOpt(w,pv),w,pv.opt.T);
            pv.t = t; pv.proj; pv.W = W;
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.ns;
                if nargout > 1, g = pv.rof*pv.grad_ns; end
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
            g = pv.grad_nt - pv.grad_nr;
        end
        
        function w = robustw(pv)
            ub = ones(size(pv.Ws)); ub = ub(:);
            lb = zeros(size(pv.Ws)); lb = lb(:);
            w = 0.5*ub;
            [w,f,exitflag,output] = fmincon(@(w)mvOpt(w,pv),w,[],[],[],[],lb,ub,[],pv.opt.W);
            pv.W = reshape(w,size(pv.W)); w = pv.W;
            if ~isequal(pv.opt.W.Display,'off'),
                output
                exitflag
                pv.disp
            end
            
            function p = mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                for k=1:pv.n, pv.sv(k).W = pv.W(:,:,k); end
                pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
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
            pv.p = p;
        end
        
        function [G2,Gx,Gy] = gradient(pv)
            % Gx, Gy: gradients with constant weight
            % G2: weighted gradient
            pv.Gs=pv.a(pv.Z).*pv.Is+pv.b(pv.Z).*pv.Ws;
            pv.Gx=pv.a(pv.Z).*pv.Ix+pv.b(pv.Z).*pv.Wx;
            pv.Gy=pv.a(pv.Z).*pv.Iy+pv.b(pv.Z).*pv.Wy;
            
            Gt = sum(pv.Gs,3); Wx = sum(pv.Wx,3); Wy = sum(pv.Wy,3);
            pv.Gm = Gt./pv.Wt; pv.Gm(isnan(pv.Gm)) = 0;
            
            Gx = sum(pv.Gx,3)-pv.Gm.*Wx;
            Gy = sum(pv.Gy,3)-pv.Gm.*Wy;
            
            pv.G2 = (Gx.^2+Gy.^2)./pv.Wt; pv.G2(isnan(pv.G2)) = 0;
            
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
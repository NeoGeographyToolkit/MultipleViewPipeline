classdef PatchViews < handle
    % terrain patch contains handle of single view
    properties (Constant)
        radiusMoon = 1737400;   % radius of the moon
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
        e               % elevation post
        n = 2;          % number of patches
        q               % quaternion of rotation
        r = PatchViews.radiusMoon % radial elevation
        s = 0.5;      % smoothing scale
        t = 10*[1 1]';   % correlation scale
        u = 31;         % unit length of the pixel
        z = [1 1]';     % point of interest
        w
        H
    end
    
    properties (Hidden)
        sv = SingleView;
        tv = TerainView;
        Is, Ix, Iy, Ws, Wx, Wy, W
        It              % gradient image w.r.t theta
        Ib              % original patch
        Wt, Wn          % total and normalized weight
        Ic, Wc          % corrected by linear reflectance
        n2, N, nc, is, js, ks, ia, ja, ka,
        x0, x1, x2, y0, y1, y2,
        a0, a1, a2, b0, b1, b2,
        Gb, Gs, Gm, Es, Eb  % albedos
        Gx, Gy, Gt, G2      % albedo gradients
        Ds              % disparity
        rof, sof   % reciprocals of dof
        ne, nr, ns  % degrees of freedom
        a, b;     % reflectance coefficients
        c = [0 4];         % shape parameters of exponetial power function
        lonlat          % longitude and latitude
        m = 1;          % multiplicative hypothesis
        p               % p-value of correspondences
        f, df           % squared error
        X, Y
        
        opt;            % optimization settings
    end
    
    methods
        function pv = PatchViews(sv,tv)
            if nargin == 2,
                addlistener(pv,'n','PostSet',@pv.PropEvents);
                pv.tv=tv; pv.sv=sv; pv.n=numel(sv);
                for i=1:pv.n,
                    obj = sv(i);
                    addlistener(pv,'e','PostSet',@obj.PropEvents);
                    addlistener(pv,'q','PostSet',@obj.PropEvents);
                    addlistener(pv,'r','PostSet',@obj.PropEvents);
                    addlistener(pv,'s','PostSet',@obj.PropEvents);
                    addlistener(pv,'w','PostSet',@obj.PropEvents);
                end
                addlistener(pv,'s','PostSet',@pv.PropEvents);
                addlistener(pv,'t','PostSet',@pv.PropEvents);
                addlistener(pv,'z','PostSet',@pv.PropEvents);
                
                % initialize SingleView objects
                pv.a = ones(pv.n,1);
                pv.b = zeros(pv.n,1);
                pv.r = pv.r;
                pv.s = pv.s;
                pv.t = pv.t;
                pv.z = [1 1]';
                
                pv.opt.R = optimset('disp','iter','Largescale','off');
                pv.opt.R = optimset(pv.opt.R,'Largescale','off');
                pv.opt.W = optimset('disp','iter','Largescale','off');
                pv.opt.W = optimset(pv.opt.W,'TolX',1e-25,'FinDiffType','central');
            end
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
        
        function c = slote(pv)  % sloppy refine
            pv.corelate;
            c = [];
            for k = 1:pv.n
                pv.sv(k).a = pv.a(k);
                pv.sv(k).b = pv.b(k);
                c = [c pv.sv(k).slote(pv.Gm)];
            end
%             pv.corelate;
%             pv.residual;
%             pv.gradient;
% 
%             s = sqrt(pv.c);
%             [s,f,exitflag,output] = fminunc(@(s)mvOpt(s,pv),s,pv.opt.R);
%             pv.c = s^2; c = s^2;
%             [p,t] = pv.slopate; 
%             pv.proj;
%             if ~isequal(pv.opt.R.Display,'off'),
%                 output
%                 exitflag
%                 pv.disp
%             end
%             function [f,p]=mvOpt(s,pv)
%                 pv.c = s^2;
%                 p=pv.slopate;
%                 if p<realmin, f = PatchViews.eps_log; else f = log(p); end
%             end
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
        
        function m = optimize(pv)
            r=1737400-3000;
            pv.opt.R = optimset(pv.opt.R,'OutputFcn',@mvOutFcn);
            for k=1:100
                [q,f,exitflag,output] = fminunc(@mvOptQ,q,pv.opt.R);
                if f < PatchViews.eps_log2p,
                    pv.r=r;
                    m = pv.adjustHypothesis;
                    fprintf('H0: var_s = %f var_e !!!\n',m);
                else
                    fprintf('%d iterations \n',k);
                    break,
                end
            end
            
            function p=mvOpt(q,r,s,t)
                pv.q=q; pv.r=r; pv.s=s; pv.t=t; pv.proj;
                p=reallog(pv.correlate+PatchViews.eps_p);
            end
        end
        
        function h = disp(pv)
            fprintf('\npoint of interest (%d,%d) in the %d*%d tile',pv.z,pv.tv.size)
            fprintf('\nlongitude and latitude (%d,%d) \n',pv.lonlat*180/pi)
            
            if ~isempty(pv.W)
                figure;
                C = num2cell(pv.W,[1 2]);
                C = reshape(C,[2 2]);
                C = cell2mat(C);
                figure, imagesc(C), axis equal, colorbar;
            end
            
            if ~isempty(pv.Is)
                pv.residual;
                A = num2cell(pv.Is,[1 2]);
                A = reshape(A,[2 2]);
                A = cell2mat(A);
                
                h = figure;
                subplot(2,3,1), imshow(imadjust(A)), axis equal
                title('Before Photometric Correction','FontSize',14)

                C = num2cell(pv.Gs,[1 2]);
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
                case 'n'
                    pv.initIndices;
                case 't'
                    pv.initWindows;
                case 'z'
                    [pv.e d pv.lonlat]=pv.tv.post(pv.z);
                    R = [d(:,1)/norm(d(:,1)) -d(:,2) -pv.e];
                    pv.q = dcm2q(R)';
            end
        end
        
        function initWindows(pv)
            w = ceil([pv.t(1:2)*PatchViews.ratioScale; pv.s*PatchViews.ratioSmoth]);
            if ~isequal(pv.w,w), pv.w = w; end
            pv.sof=diff(normcdf([-0.5 0.5],0,pv.s))^2; % smoothing compensation
            [pv.x0 pv.x1 pv.x2] = wndGaussian(pv.w(1),pv.t(1));
            [pv.y0 pv.y1 pv.y2] = wndGaussian(pv.w(2),pv.t(2));
            pv.rof = pv.x0(pv.w(1)+1)*pv.y0(pv.w(2)+1); % reciprocal of dof
            [pv.X, pv.Y]=meshgrid(-pv.w(1):pv.w(1),-pv.w(2):pv.w(2));
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
            pv.Is=[]; pv.Ix=[]; pv.Iy=[]; pv.It=[];
            pv.Ws=[]; pv.Wx=[]; pv.Wy=[]; pv.Ib=[];
            for k=1:pv.n
                [pv.Is(:,:,k),pv.Ws(:,:,k),pv.Ix(:,:,k),pv.Wx(:,:,k),...
                    pv.Iy(:,:,k),pv.Wy(:,:,k),pv.It(:,:,k),pv.Ib(:,:,k)] = pv.sv(k).crop;
            end
            
            % total and normalized weights
            pv.Wt = sum(pv.Ws,3);
            pv.Wn = pv.Ws./repmat(pv.Wt,[1 1 pv.n]);
            pv.Wn(find(isnan(pv.Wn))) = 0;
            
            if nargout > 0, I = pv.Is; end
            if nargout > 1, W = pv.Ws; end
        end
        
        function E = residual(pv)
            pv.Gs=[]; pv.Gb=[];
            for k=1:pv.n
                pv.Gs(:,:,k) = pv.Is(:,:,k)*pv.a(k)+pv.b(k);
                pv.Gb(:,:,k) = pv.Ib(:,:,k)*pv.a(k)+pv.b(k);
            end
            Wt = sum(pv.Ws,3);
            pv.Gm = sum(pv.Ws.*pv.Gs,3)./Wt;
            pv.Gm(find(isnan(pv.Gm))) = 0;
            pv.Es = pv.Gs - pv.Gm(:,:,ones(pv.n,1));
            pv.Eb = pv.Gb - pv.Gm(:,:,ones(pv.n,1));
            if nargout > 0, E = pv.Eb; end
        end
        
        function [f,a,b] = phometry(pv)
            % Scatter Matrices
            Iw = pv.Ws.*pv.Is; In = pv.Wn.*pv.Is;
            Ea = scatw(Iw(:,:,pv.is).*In(:,:,pv.js),Iw.*pv.Is,pv.ks,pv.x0,pv.y0);
            Eb = scatw(pv.Wn(:,:,pv.is).*pv.Ws(:,:,pv.js),pv.Ws,pv.ks,pv.x0,pv.y0);
            Ec = scatw(Iw(:,:,pv.ia).*pv.Wn(:,:,pv.ja),Iw,pv.ka,pv.x0,pv.y0);
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
            Gs = pv.Ws.*pv.Gs;  % smooth image
            Gx = pv.Ws.*pv.Gx; Gy = pv.Ws.*pv.Gy; Gt = pv.Ws.*pv.Gt; % weighted gradients
            Nx = pv.Wn.*pv.Gx; Ny = pv.Wn.*pv.Gy; Nt = pv.Wn.*pv.Gt; % normalized gradients
            %  symmetric components of Hessian
            Exx = scatw(Nx(:,:,pv.is).*Gx(:,:,pv.js),Gx.*pv.Gx,pv.ks,pv.x0,pv.y0);
            Eyy = scatw(Ny(:,:,pv.is).*Gy(:,:,pv.js),Gy.*pv.Gy,pv.ks,pv.x0,pv.y0);
            Ett = scatw(Nt(:,:,pv.is).*Gt(:,:,pv.js),Gt.*pv.Gt,pv.ks,pv.x0,pv.y0);
            % asymmetric components of Hessian
            Exy = scatw(Nx(:,:,pv.ia).*Gy(:,:,pv.ja),Gx.*pv.Gy,pv.ka,pv.x0,pv.y0);
            Ext = scatw(Nx(:,:,pv.ia).*Gt(:,:,pv.ja),Gx.*pv.Gt,pv.ka,pv.x0,pv.y0);
            Eyt = scatw(Ny(:,:,pv.ia).*Gt(:,:,pv.ja),Gy.*pv.Gt,pv.ka,pv.x0,pv.y0);
            % gradient components
            Gx = scatw(Nx(:,:,pv.ia).*Gs(:,:,pv.ja),Gx.*pv.Gs,pv.ka,pv.x0,pv.y0);
            Gy = scatw(Ny(:,:,pv.ia).*Gs(:,:,pv.ja),Gy.*pv.Gs,pv.ka,pv.x0,pv.y0);
            Gt = scatw(Nt(:,:,pv.ia).*Gs(:,:,pv.ja),Gt.*pv.Gs,pv.ka,pv.x0,pv.y0);
            
            H = [Exx Exy Ext; Exy' Eyy Eyt; Ext' Eyt' Ett];
            g = sum([Gx; Gy; Gt],2);        % gradient 
            w = wnd3(pv.y0,pv.x0,pv.Ws)';   % individual weights

            z = zeros(1,pv.n);
            A = [w z z; z w z; z z w];
            b = zeros(3,1); t0 = zeros(3*pv.n,1);
            
            options = optimset('Display','off');
            [t,fval,exitflag] = quadprog(H,g,[],[],A,b,[],[],t0,options);
            t = reshape(t,pv.n,3);
            
            f = pv.f;       % current squared error
            for k = 1:pv.n
                pv.sv(k).h = pv.sv(k).h + t(k,:)';
            end
            
            pv.proj
            pv.phometry;
            if f < pv.f,    % check smaller squared error 
                for k = 1:pv.n
                    pv.sv(k).h = pv.sv(k).h - t(k,:)';
                end
                fprintf('<');
            else
            end
        end
        
        function t = geoptimi(pv)
            t = zeros(3*pv.n,1); z = zeros(1,pv.n);
            w = wnd3(pv.y0,pv.x0,pv.Ws)';    % individual weights
            A = [w z z; z w z; z z w];
            b = zeros(3,1);

            [t,f,exitflag,output] = fmincon(@(t)mvOpt(t,pv),t,[],[],A,b,[],[],[],pv.opt.R);
            t = reshape(t,pv.n,3);
            for k = 1:pv.n, pv.sv(k).h = -t(k,:)'; end
            pv.proj;
            if ~isequal(pv.opt.R.Display,'off'),
                output
                exitflag
                pv.disp
            end
            
            function p = mvOpt(t,pv)
                t = reshape(t,pv.n,3);
                for k = 1:pv.n, pv.sv(k).h = -t(k,:)'; end
                pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
        end

        function c = robust(pv)
            E = pv.residual;
            if pv.c(1) == 0 
                pv.c(1) = 1/std(E(:)); 
%             else
%                 pv.c(1) = pv.c(1)/2;
            end
            [c,f,exitflag,output] = fminunc(@(c)mvOpt(c,pv),pv.c,pv.opt.W);
%             E = pv.residual; lb = -max(abs(E(:)));
%             [c,f,exitflag,output] = fminbnd(@(c)mvOpt(c,pv),lb,0,pv.opt.W);
            if ~isequal(pv.opt.W.Display,'off'),
                output
                exitflag
                pv.disp
            end
            
%             function p = mvOpt(c,pv)
%                 E = pv.residual;
%                 pv.W = ones(size(E));
%                 pv.W(find(abs(E)>-c))=0;
%                 for k=1:pv.n, pv.sv(k).W = pv.W(:,:,k); end
%                 pv.proj;
%                 p=reallog(pv.corelate+PatchViews.eps_p);
%             end
            function p = mvOpt(c,pv)
                E = pv.residual;
                pv.W = exp(-(abs(E)*c(1)).^c(2));
                for k=1:pv.n, pv.sv(k).W = pv.W(:,:,k); end
                pv.proj;
                p=reallog(pv.corelate+PatchViews.eps_p);
            end
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
            [f,a,b]=pv.phometry;     % photometric estimation
            pv.gradient;     % gradient estimation
%            pv.geometry;     % gradient estimation
            
            % Confidence Value
            s = wnd3(pv.y0,pv.x0,pv.G2);    % signal
            w = wnd3(pv.y0,pv.x0,pv.Ws);    % individual weights
            pv.ns = pv.sof*sum(wnd3(pv.y0,pv.x0,pv.Wn.*pv.Ws))/pv.rof/2;
            pv.ne = pv.sof*sum(w)/pv.rof/2-pv.ns-2;
            if any([s pv.ns pv.ne] < 0),
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
            pv.Gs=[]; pv.Gt=[]; pv.Gx=[]; pv.Gy=[];
            for k=1:pv.n
                pv.Gs(:,:,k)=pv.a(k)*pv.Is(:,:,k)+pv.b(k);
                pv.Gx(:,:,k)=pv.a(k)*pv.Ix(:,:,k);
                pv.Gy(:,:,k)=pv.a(k)*pv.Iy(:,:,k);
                pv.Gt(:,:,k)=pv.a(k)*pv.It(:,:,k);
            end
            Gw = pv.Gs.*pv.Ws;
            Gx = pv.Gx.*pv.Ws+pv.Gs.*pv.Wx;
            Gy = pv.Gy.*pv.Ws+pv.Gs.*pv.Wy;
                
            Gt = sum(Gw,3); Wx = sum(pv.Wx,3); Wy = sum(pv.Wy,3);
            Gx = pv.Wt.*sum(Gx,3)-Gt.*Wx; Gx(find(isnan(Gx))) = 0;
            Gy = pv.Wt.*sum(Gy,3)-Gt.*Wy; Gy(find(isnan(Gy))) = 0;
            pv.G2 = (Gx.^2+Gy.^2)./pv.Wt.^3; pv.G2(find(isnan(pv.G2))) = 0;
            
            if nargout > 0, Gx = pv.Gx; end
            if nargout > 1, Gy = pv.Gy; end
            if nargout > 2, G2 = pv.G2; end
        end
        
        function [p,t] = slopate(pv,s)
            if nargin < 2, r2 = pv.c^2; else r2 = s^2; end
           
            D = pv.G2+1/r2;
            tx = pv.Es.*repmat(pv.Gx./D,[1 1 pv.n]);
            ty = pv.Es.*repmat(pv.Gy./D,[1 1 pv.n]);
%            s = wnd3(pv.y0,pv.x0,pv.Wt.*G2);
%             dx = round(tx); 
%             dx(find(dx >  pv.w(3))) =  pv.w(3);
%             dx(find(dx < -pv.w(3))) = -pv.w(3);
%             dy = round(ty);
%             dy(find(dy >  pv.w(3))) =  pv.w(3);
%             dy(find(dy < -pv.w(3))) = -pv.w(3);
%             s0 = wndGaussian(pv.w(3),pv.s);
%             Ws = (s0(dx+pv.w(3)+1).*s0(dy+pv.w(3)+1)).*pv.Ws;
            t2 = tx.^2+ty.^2;
            Ws = r2*exp(-t2/2/pv.s^2).*pv.Ws.*repmat(pv.G2./D,[1 1 pv.n]);
            w = wnd3(pv.y0,pv.x0,Ws);
            
            R2 = pv.X.^2+pv.Y.^2;
            t(:,1) = wnd3(pv.y0,pv.x0,Ws.*tx)./w;
            t(:,2) = wnd3(pv.y0,pv.x0,Ws.*ty)./w;
            E = wnd3(pv.y0,pv.x0,Ws.*pv.Es.^2./repmat(D.^2,[1 1 pv.n]))/r2;
            wtx = wnd3(pv.y0,pv.x0,Ws.*tx);
            wty = wnd3(pv.y0,pv.x0,Ws.*ty);
            wr = wnd3(pv.y0,pv.x0,Ws.*repmat(R2,[1 1 pv.n]));
            zx = wnd3(pv.y0,pv.x0,Ws.*repmat(pv.X,[1 1 pv.n]));
            zy = wnd3(pv.y0,pv.x0,Ws.*repmat(pv.Y,[1 1 pv.n]));
            wyx = wnd3(pv.y0,pv.x0,Ws.*ty.*repmat(pv.X,[1 1 pv.n]));
            wxy = wnd3(pv.y0,pv.x0,Ws.*tx.*repmat(pv.Y,[1 1 pv.n]));
            d(:,1) = wyx-wxy-zx.*t(:,2)+zy.*t(:,1);
            wtx = wnd3(pv.y0,pv.x0,Ws.*tx.*repmat(pv.X,[1 1 pv.n]));
            wty = wnd3(pv.y0,pv.x0,Ws.*ty.*repmat(pv.Y,[1 1 pv.n]));
            d(:,2) = wr+sum([wtx wty]-t.*[zx zy],2)+w.*sum(t.^2,2);
            t(:,3) = atan2(d(:,1),d(:,2));
            
            Wt = sum(Ws,3); sw = sum(w);
            Wn = Ws./repmat(Wt,[1 1 pv.n]);
            ns = pv.sof*sum(wnd3(pv.y0,pv.x0,Wn.*Ws))/pv.rof/2;
            nr = (sw-w'*w/sw)/max(w);
            ne = pv.sof*sw/pv.rof/2-ns-nr;
            
            d2 = wnd3(pv.y0,pv.x0,Ws.*t2);
            C = sqrt(d(:,1).^2+d(:,2).^2);
            S = 2*(d(:,2)-C)+3*w.*sum(t(:,1:2).^2,2)+d2;            
            [p p1 p2]= fbval(sum(S),2*sum(E),2*ne,ne);
        end
        
        function [p,a,b] = correlate(pv)
            % Scatter Matrices
            Wn = pv.Ws./repmat(sum(pv.Ws,3),[1 1 pv.n]);
            Wn(find(isnan(Wn))) = 0;
            Iw = pv.Ws.*pv.Is; In = Wn.*pv.Is;
            [Ea,Ra] = scatw(Iw(:,:,pv.is).*In(:,:,pv.js),Iw.*pv.Is,pv.ks,pv.x0,pv.y0);
            [Eb,Rb] = scatw(Wn(:,:,pv.is).*pv.Ws(:,:,pv.js),pv.Ws,pv.ks,pv.x0,pv.y0);
            [Ec,Rc] = scatw(Iw(:,:,pv.ia).*Wn(:,:,pv.ja),Iw,pv.ka,pv.x0,pv.y0);
            wa = wnd3(pv.y0,pv.x0,In); wb = wnd3(pv.y0,pv.x0,Wn);
            T = (Eb*Eb+wb*wb')\(Eb*Ec'+wb*wa');
            Et = T'*Eb*T/2-Ec*T;
            E = Ea+Et+Et'; % error matrix with symmetry
            
            % compute the smallest generalized Eigen vector a of D
            [P,D] = eig(E);
            [pv.f,k] = min(diag(D));
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
                pv.f = a'*E*a;
                fprintf('-');
            end
            b = -T*a;
            
            % Confidence Value
            w = wnd3(pv.y0,pv.x0,pv.Ws);
            s = a'*Ra*a+2*a'*Rc*b+b'*Rb*b;
            pv.ns = pv.sof*sum(wnd3(pv.y0,pv.x0,Wn.*pv.Ws))/pv.rof/2;
            pv.ne = pv.sof*sum(w)/pv.rof/2-pv.ns-2;
            if any([s pv.ns pv.ne] < 0),
                fprintf('All values and dofs of signal and error should be non-negative\n');
                fprintf('signal: %f, dof: %f\n',s,pv.ns);
                fprintf(' error: %f, dof: %f\n',pv.f,pv.ne);
            end
            
            p = fpval(s,pv.m*pv.f,pv.ns,pv.ne);
            pv.p = p; pv.a = a; pv.b = b;
        end
        
        function set.q(obj,q) % q property set function
            obj.q = q;
        end % set.q
        
        function q=get.q(obj) % q property set function
            q = obj.q;
        end % get.q
        
        function set.z(obj,z) % z property set function
            obj.z = z;
        end % set.z
        
        function z=get.z(obj) % z property set function
            z = obj.z;
        end % get.z
        
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
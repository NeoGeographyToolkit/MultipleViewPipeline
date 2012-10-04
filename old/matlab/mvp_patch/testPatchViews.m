classdef testPatchViews < TestCase
    
    properties
        mv
        pv
        opt
    end
    
    methods
        function self = testPatchViews(name)
            self = self@TestCase(name);
        end
        
        function setUp(self)
            strIn = 'AS15_3_3_tiles.mat';
            self.mv = MultiViews(strIn);
            self.mv.raster([128 150]');
            self.pv = self.mv.rv.pv;
            self.pv.disp;
            self.opt.T = optimset('LargeScale','on','FinDiffType','central');
            self.opt.T = optimset(self.opt.T,'MaxIter',0,'DerivativeCheck','on');
            self.opt.T = optimset(self.opt.T,'GradObj','on','OutputFcn',@outfun);
            close all;
        end
        
        function tearDown(self)
            %            delete(self.mv);
        end
        
        function testGradSe(self)
            fprintf('Testing Gradient of Error\n');
            self.pv.t = [2 2]';
            self.pv.proj; self.pv.corelate;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.slowate;
                f = pv.se;
                if nargout > 1, g = pv.grad_se; end
            end
        end
        
        function testGs(self)
            fprintf('Testing Gs\n');
            self.pv.t = [2 2]'; self.pv.proj;
            self.pv.slowate;
            Gs = self.pv.Gs;
            Gb = [];
            for k = 1:self.pv.n
                Gb(:,:,k) = self.pv.a(k)*self.pv.Fy(:,:,k)+self.pv.b(k);
            end
            Wb = self.pv.W.*self.pv.Wb;
            Gw = Gb.*Wb;
            Gt = convn(Gw,self.pv.sv(1).s0*self.pv.sv(1).s0','same');
            assertElementsAlmostEqual(Gb,self.pv.Gb);
            assertElementsAlmostEqual(Gs,Gt);
        end
        
        function testGradP(self)
            fprintf('Testing Gradient of p-value\n');
            self.pv.t = [2 2]';
            self.pv.proj; self.pv.corelate;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.slowate;
                f = pv.p;
                if nargout > 1, g = pv.grad_p; end
            end
        end
        
        function testGradBeta(self)
            fprintf('Testing Gradient of Beta\n');
            self.pv.proj; self.pv.corelate;
            l = [0 0 0]'; u = [1 inf inf]';
            z = [self.pv.se/(self.pv.se+self.pv.ss) self.pv.ne self.pv.ns];
            %             x = [self.pv.ne/(self.pv.ne+self.pv.ns) self.pv.ne self.pv.ns];
            fmincon(@(z)mvOpt(z,self.pv),z,[],[],[],[],l,u,[],self.opt.T);
            g = mvGrad(z,self.pv)
            
            function g = mvGrad(X,pv,varargin)
                LB = [0 0 0]'; UB = [1 inf inf]';
                g = gradest(@(X)mvOpt(X,pv),X,LB,UB,pv.opt.T);
            end
            
            function [f,g]=mvOpt(z,pv)
                a = z(2); b = z(3); x = z(1);
                f = betainc(x,a,b);
                if nargout > 1,
                    g = mvGrad(z,pv);
%                     c = a+b; y = 1-x;
%                     pa = psi(a); pb = psi(b); pc = psi(c);
%                     ga = gamma(a); gb = gamma(b); gc = gamma(c);
%                     la = gammaln(a); lb = gammaln(b); lc = gammaln(c);
%                     Fx = hypergeom([a a 1-b],[a+1 a+1],x);
%                     Fy = hypergeom([b b 1-a],[b+1 b+1],y);
%                     g(1) = y^(b-1)*x^(a-1)/beta(a,b);
%                     Dx = 0; %exp(la+lb-lc+a*log(x)+log(Fx)); % exp(la+lc-lb+a*log(x)+log(Fx));
%                     g(2) = (log(x)-pa+pc)*f-Dx;
%                     Dy = 0; %exp(la+lb-lc+b*log(y)+log(Fy)); % exp(lb+lc-la+b*log(y)+log(Fy));
%                     g(3) = Dy-(log(y)-pb+pc)*f;
                end
            end
        end
        
        function testGradBetaZ(self)
            fprintf('Testing Gradient of Beta Z\n');
            self.pv.proj;
            x = self.pv.ne/(self.pv.ne+self.pv.ns); % self.pv.se/(self.pv.se+self.pv.ss);
            fmincon(@(x)mvOpt(x,self.pv),x,[],[],[],[],0,1,[],self.opt.T);
            function [f,g]=mvOpt(x,pv)
                a = pv.ne; b = pv.ns;
                f = betainc(x,a,b);
                if nargout > 1,
                    c = a+b; y = 1-x;
                    pa = psi(a); pb = psi(b); pc = psi(c);
                    ga = gamma(a); gb = gamma(b); gc = gamma(c);
                    la = gammaln(a); lb = gammaln(b); lc = gammaln(c);
                    Fx = hypergeom([a a 1-b],[a+1 a+1],x);
                    Fy = hypergeom([b b 1-a],[b+1 b+1],y);
                    g(1) = y^(b-1)*x^(a-1)/beta(a,b);
                end
            end
        end
        
        function testGradGs(self)
            fprintf('Testing Gradient of Gs\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.gs;
                if nargout > 1, g = pv.grad_gs; end
            end
        end
        
        function testDNtDQ(self)
            fprintf('Testing Gradient of Ns w.r.t r\n');
            p = self.pv.r;
            fminunc(@(p)mvOpt(p,self.pv),p,self.opt.T);
            function [f,g]=mvOpt(p,pv)
                pv.r = p;
                pv.proj;
                pv.corelate;
                f = pv.nt;
                if nargout > 1, 
                    pv.jacobian;
                    g = pv.dNtdQ;
                end
            end
        end
        
        function testGGN(self)
            fprintf('Testing Generalized Gauss-Newton w.r.t r\n');
            p = self.pv.optimize;
        end
        
        function testDNsDQ(self)
            fprintf('Testing Gradient of Ns w.r.t r\n');
            p = self.pv.r;
            fminunc(@(p)mvOpt(p,self.pv),p,self.opt.T);
            function [f,g]=mvOpt(p,pv)
                pv.r = p;
                pv.proj;
                pv.corelate;
                f = pv.ns;
                if nargout > 1, 
                    pv.jacobian;
                    g = pv.dNsdQ; 
                end
            end
        end
        
        function testDSeDQ(self)
            fprintf('Testing Gradient of Ns w.r.t r\n');
            p = self.pv.r;
            fminunc(@(p)mvOpt(p,self.pv),p,self.opt.T);
            function [f,g]=mvOpt(p,pv)
                pv.r = p;
                pv.proj;
                pv.corelate;
                f = pv.se;
                if nargout > 1, 
                    pv.jacobian;
                    g = pv.dSedQ; 
                end
            end
        end
        
        function testDSsDQ(self)
            fprintf('Testing Gradient of Ns w.r.t r\n');
            p = self.pv.r;
            fminunc(@(p)mvOpt(p,self.pv),p,self.opt.T);
            function [f,g]=mvOpt(p,pv)
                pv.r = p;
                pv.proj;
                pv.corelate;
                f = pv.ss;
                if nargout > 1, 
                    pv.jacobian;
                    g = pv.dSsdQ; 
                end
            end
        end
        
        function testGradMs(self)
            fprintf('Testing Gradient of Ms\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.ms;
                if nargout > 1, g = pv.grad_ms; end
            end
        end
        
        function testGradMx(self)
            fprintf('Testing Gradient of Mx\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.mx;
                if nargout > 1, g = pv.grad_mx; end
            end
        end
        
        function testGradMy(self)
            fprintf('Testing Gradient of My\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.my;
                if nargout > 1, g = pv.grad_my; end
            end
        end
        
        function testGradGx(self)
            fprintf('Testing Gradient of Gx\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.gx;
                if nargout > 1, g = pv.grad_gx; end
            end
        end
        
        function testGradGy(self)
            fprintf('Testing Gradient of Gy\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:); self.pv.corelate;
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.gy;
                if nargout > 1, g = pv.grad_gy; end
            end
        end
        
        function testGradWs(self)
            fprintf('Testing Gradient of Ws\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.slowate;
                f = pv.ws;
                if nargout > 1, g = pv.grad_ws; end
            end
        end
        
        function testGradWx(self)
            fprintf('Testing Gradient of Wx\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.slowate;
                f = pv.wx;
                if nargout > 1, g = pv.grad_wx; end
            end
        end
        
        function testGradWy(self)
            fprintf('Testing Gradient of Wy\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj;
                pv.slowate;
                f = pv.wy;
                if nargout > 1, g = pv.grad_wy; end
            end
        end
        
        function testGradSs(self)
            fprintf('Testing Gradient of Signal\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            [f,g]=mvOpt(w,self.pv); g = reshape(g,size(self.pv.Ws));
            figure, imagesc(g(:,:,1)), colorbar;
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.slowate;
                f = pv.ss;
                if nargout > 1, g = pv.grad_ss; end
            end
        end
        
        function testGradNt(self)
            fprintf('Testing Gradient of Total DoF\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.nt;
                if nargout > 1, g = pv.rof*pv.grad_nt; end
            end
        end
        
        function testGradNs(self)
            fprintf('Testing Gradient of Signal DoF\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.ns;
                if nargout > 1, g = pv.rof*pv.grad_ns; end
            end
        end
        
        function testGradNe(self)
            fprintf('Testing Gradient of Error DoF\n');
            self.pv.t = [2 2]'; self.pv.proj;
            w = self.pv.W(:);
            [w,f,exitflag,output] = fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.ne;
                if nargout > 1, g = pv.rof*pv.grad_ne; end
            end
        end
        
        %         function testColormapColumns(self)
        %             self.mv.disp
        % %            assertEqual(size(get(self.fh, 'Colormap'), 2), 3);
        %         end
        %
        %         function testPointer(self)
        %             self.mv.raster([128 150]')
        % %            assertEqual(get(self.fh, 'Pointer'), 'arrow');
        %         end
    end
end

function stop = outfun(x,optimValues,state)
stop = true;
end
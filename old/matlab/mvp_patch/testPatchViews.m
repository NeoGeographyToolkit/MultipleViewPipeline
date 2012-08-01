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
            self.mv.rv.z=[128 150]';
            self.mv.rv.pv.proj
            r = self.mv.rv.elevate(RasterView.radiusMoon-500);
            self.pv = self.mv.rv.pv;
            self.pv.disp;
            self.opt.T = optimset('LargeScale','off','FinDiffType','central');
            self.opt.T = optimset(self.opt.T,'MaxIter',0,'DerivativeCheck','on');
            self.opt.T = optimset(self.opt.T,'GradObj','on','OutputFcn',@outfun);
            close all;
        end

        function tearDown(self)
%            delete(self.mv);
        end

        function testGradSe(self)
            fprintf('Testing Gradient of Error\n');
            self.pv.t = [1 1]';
            self.pv.proj; self.pv.corelate;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.se;
                if nargout > 1, g = pv.grad_se; end
            end
        end
        
        function testGradSs1(self)
            fprintf('Testing Gradient of Signal 1\n');
            self.pv.t = [1 1]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; 
                pv.slowate;
                f = pv.s1;
                if nargout > 1, g = pv.grad_ss1; end
            end
        end
        
        function testGs(self)
            fprintf('Testing Gs\n');
            self.pv.t = [1 1]'; self.pv.proj;
            self.pv.slowate;
            Gs = self.pv.Gs;
            Gb = [];
            for k = 1:self.pv.n
                Gb(:,:,k) = self.pv.a(k)*self.pv.Fb(:,:,k)+self.pv.b(k);
            end
            Wb = self.pv.W.*self.pv.Wb;
            Gw = Gb.*Wb;
            Gt = convn(Gw,self.pv.sv(1).s0*self.pv.sv(1).s0','same');
            assertElementsAlmostEqual(Gb,self.pv.Gb);
            assertElementsAlmostEqual(Gs,Gt);
        end
        
        function testGradGs(self)
            fprintf('Testing Gradient of Gs\n');
            self.pv.t = [1 1]'; self.pv.proj;
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
        
        function testGradMs(self)
            fprintf('Testing Gradient of Ms\n');
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; 
                pv.slowate;
                f = pv.ws;
                if nargout > 1, g = pv.grad_ws; end
            end
        end
        
        function testGradWx(self)
            fprintf('Testing Gradient of Wx\n');
            self.pv.t = [1 1]'; self.pv.proj;
            w = self.pv.W(:);
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; 
                pv.slowate;
                f = pv.wx;
                if nargout > 1, g = pv.grad_wx; end
            end
        end
        
        function testGradWy(self)
            fprintf('Testing Gradient of Wy\n');
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj;
            w = self.pv.W(:);
            [f,g]=mvOpt(w,self.pv); g = reshape(g,size(self.pv.Ws));
            figure, imagesc(g(:,:,1)), colorbar;
            fminunc(@(w)mvOpt(w,self.pv),w,self.opt.T);
            function [f,g]=mvOpt(w,pv)
                pv.W = reshape(w,size(pv.Ws));
                pv.proj; pv.corelate;
                f = pv.ss;
                if nargout > 1, g = pv.grad_ss; end
            end
        end
        
        function testGradNt(self)
            fprintf('Testing Gradient of Total DoF\n');
            self.pv.t = [1 1]'; self.pv.proj;
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
            self.pv.t = [1 1]'; self.pv.proj; 
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
            self.pv.t = [1 1]'; self.pv.proj; 
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
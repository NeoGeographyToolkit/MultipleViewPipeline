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
            self.mv.rv.z=[128 128]';
            self.mv.rv.pv.proj
            r = self.mv.rv.elevate(RasterView.radiusMoon-500);
            h = r-RasterView.radiusMoon
            self.pv = self.mv.rv.pv;
            self.pv.disp;
            self.opt.T = optimset('LargeScale','off','FinDiffType','central');
            self.opt.T = optimset(self.opt.T,'MaxIter',0,'DerivativeCheck','on');
            self.opt.T = optimset(self.opt.T,'GradObj','on');
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
        
        function testGradSs(self)
            fprintf('Testing Gradient of Error\n');
            self.pv.t = [1 1]'; self.pv.proj;
            w = self.pv.W(:);
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

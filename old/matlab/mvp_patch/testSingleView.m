classdef testSingleView < TestCase

    properties
        mv
        sv
        opt
    end

    methods
        function self = testSingleView(name)
            self = self@TestCase(name);
        end

        function setUp(self)
            strIn = 'AS15_3_3_tiles.mat';
            self.mv = MultiViews(strIn);
            self.mv.raster([128 128]');
            self.mv.rv.pv.s = 1;
            self.sv = self.mv.rv.sv(1);
%             self.opt.T = optimset('Largescale','off','FinDiffType','central');
%             self.opt.T = optimset(self.opt.T,'MaxIter',0,'DerivativeCheck','on');
%             self.opt.T = optimset(self.opt.T,'GradObj','on');
            close all
        end

        function tearDown(self)
            delete(self.mv);
        end

        function testProj(self)
            a = 1e-3; b = 1e-3; c = 1; d = 8e-6; 
            w = self.sv.w(3)+1; r = 0;
            ens = abs((c^3/c-(c-d)^3/(c-d))/d-(2*c-d));   % numerical error
            ey = 10*max([ens d]);
            [Ns0,Nr,Nx,Ny,Nxr,Nyr] = self.sv.reset(a,b,c+d);
            [Ns1,Nr,Nx,Ny,Nxr,Nyr] = self.sv.reset(a,b,c-d);
            [Na,Nr,Nx,Ny,Nxr,Nyr] = self.sv.reset(a,b,c);
            [Fa0,Na0,Fx0,Nx0,Fy0,Ny0,Fb0,Nb0] = self.sv.crop;
            [dFa,dNa,dFx,dNx,dFy,dNy] = self.sv.grad_r;
            self.sv.r = r-d; self.sv.elevate;
            [Fa0,Na0,Fx0,Nx0,Fy0,Ny0,Fb0,Nb0] = self.sv.crop;
            self.sv.r = r+d; self.sv.elevate;
            [Fa1,Na1,Fx1,Nx1,Fy1,Ny1,Fb1,Nb1] = self.sv.crop;
            DFa = (Fa1-Fa0)/d/2; DNa = (Na1-Na0)/d/2;
            DFx = (Fx1-Fx0)/d/2; DNx = (Nx1-Nx0)/d/2;
            DFy = (Fy1-Fy0)/d/2; DNy = (Ny1-Ny0)/d/2;
            DFb = (Fb1-Fb0)/d/2; DNb = (Nb1-Nb0)/d/2;
            DNs = (Ns1-Ns0)/d/2;
            figure, imagesc(DNs), colorbar
            figure, imagesc(Nr), colorbar
            figure, imagesc(DNs-Nr), colorbar
            sz = size(dFa); szm = round(sz/2); i = szm(1); j = szm(2);
            I = w:sz(1)-w+1; J = w:sz(2)-w+1;
            x = self.sv.X(i,J); y = self.sv.Y(I,j);
            figure, plot(x,Nb0(i,J),'r',x,Na(i,J),'b')
            figure, plot(x,Nr(i,J),'b',x,dNa(i,J),'r')
            figure, plot(x,dNa(i,J),'r',x,DNa(i,J),'b')
%             figure, surf(y,x,Nx0(I,J)), shading interp, colorbar, view(0,90)
%             figure, surf(y,x,Nx0(I,J)-Nx0(I,J)), shading interp, colorbar, view(0,90)
%             figure, plot(x,abs(DNa(i,J)-dNa(i,J)),'r'), ylim([0 ey]);
%            figure, imagesc(dNa(I,J)), colorbar
            er = abs((DNa(i,j)-dNa(i,j))-(d-2*c));   % numerical error

            assertElementsAlmostEqual(DNa(I,J),dNa(I,J),'absolute',ey)
            assertElementsAlmostEqual(DNx(I,J),dNx(I,J),'absolute',ey)
            assertElementsAlmostEqual(DNy(I,J),dNy(I,J),'absolute',ey)
            assertElementsAlmostEqual(DFa(I,J),dFa(I,J),'absolute',ey)
            assertElementsAlmostEqual(DFx(I,J),dFx(I,J),'absolute',ey)
            assertElementsAlmostEqual(DFy(I,J),dFy(I,J),'absolute',ey)
        end
        
        function testGradR(self)
            d = 5e-3; r = self.sv.r; w = self.sv.w(3)+1;
            self.sv.img = checkerboard(20,20,20); self.sv.crop;
            [dFa,dNa,dFx,dNx,dFy,dNy] = self.sv.grad_r;
            self.sv.r = r-d; self.sv.elevate;
            [Fa0,Na0,Fx0,Nx0,Fy0,Ny0] = self.sv.crop;
            self.sv.r = r+d; self.sv.elevate;
            [Fa1,Na1,Fx1,Nx1,Fy1,Ny1] = self.sv.crop;
            DFa = (Fa1-Fa0)/d/2; DNa = (Na1-Na0)/d/2;
            DFx = (Fx1-Fx0)/d/2; DNx = (Nx1-Nx0)/d/2;
            DFy = (Fy1-Fy0)/d/2; DNy = (Ny1-Ny0)/d/2;
            sz = size(dFa); szm = round(sz/2); i = szm(1); j = szm(2);
            I = w:sz(1)-w+1; J = w:sz(2)-w+1;
            x = self.sv.X(i,J); y = self.sv.Y(I,j);
            figure, imagesc(DFa(I,J)-dFa(I,J)), colorbar
            figure, imagesc(dFa(I,J)), colorbar
            assertElementsAlmostEqual(DNa(I,J),dNa(I,J))
            assertElementsAlmostEqual(DNx(I,J),dNx(I,J))
            assertElementsAlmostEqual(DNy(I,J),dNy(I,J))
            assertElementsAlmostEqual(DFa(I,J),dFa(I,J),'absolute',2e-4)
            assertElementsAlmostEqual(DFx(I,J),dFx(I,J),'absolute',5e-5)
            assertElementsAlmostEqual(DFy(I,J),dFy(I,J),'absolute',5e-5)
%            self.sv.disp
        end
        
                    %             D = abs(sv.H(3,3)^3/det(sv.H)); % determinant of jacobian
            %             if abs(sv.Nb(sv.yb(2)+1,sv.xb(2)+1)-D) > SingleView.eps_sqrt
            %                 figure, imagesc(sv.Nb), colorbar
            %                 error('Jacobian should be similar to numerical estimate!!!')
            %             end

    end
end

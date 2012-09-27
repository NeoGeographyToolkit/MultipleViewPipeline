classdef testRasterView < TestCase
    
    properties
        mv
        rv
    end
    
    methods
        function self = testRasterView(name)
            self = self@TestCase(name);
        end
        
        function setUp(self)
            strIn = 'AS15_3_3_tiles.mat';
            self.mv = MultiViews(strIn);
            self.rv = self.mv.rv;
            close all;
        end
        
        function tearDown(self)
            %            delete(self.mv);
        end
        
        function testRaster(self)
            self.rv.z = [128 150]';
            self.rv.pv.proj
            r = self.rv.elevate(RasterView.radiusMoon-500);
            h = r-RasterView.radiusMoon
            
            q = self.rv.rotate
            for k=1:5
                k
                [p(k),~,~,f(k)] = self.rv.pv.corelate;
                t = self.rv.pv.geometry
                self.rv.pv.proj
            end
            figure, plot(log(f))
            figure, plot(log(p))
            self.rv.disp
            t = self.rv.scate
            s = self.rv.smote
            for k=1:5
                k
                [p(k),~,~,f(k)] = self.rv.pv.corelate;
                t = self.rv.pv.geometry
                self.rv.pv.proj
            end
            figure, plot(log(f))
            figure, plot(log(p))
            c = self.rv.pv.robust
            self.rv.disp
        end
    end
end
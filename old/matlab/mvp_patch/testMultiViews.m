classdef testMultiViews < TestCase
    
    properties
        mv
    end
    
    methods
        function self = testMultiViews(name)
            self = self@TestCase(name);
            % compile with
%        mex -DMEXPRINTF asa_wrapper.c ASA_CG-2.2/asa_cg.c -IASA_CG-2.2 -largeArrayDims -v
        end
        
        function setUp(self)
            strIn = 'AS15_3_3_tiles.mat';
            self.mv = MultiViews(strIn);
            close all
        end
        
        function tearDown(self)
            delete(self.mv);
        end
        
        function testDisplay(self)
            self.mv.disp
            %            assertEqual(size(get(self.fh, 'Colormap'), 2), 3);
        end
        
        function testRaster(self)
            strIn = 'AS15_3_3_tiles.mat';
            self.mv = MultiViews(strIn);
            self.mv.raster([128 150]');
        end
        
        function testConstructor(self)
            strIn = 'AS15_3_3_tiles.mat';
            self.mv = MultiViews(strIn);
            radiusMoon = 1737400;
            lb = radiusMoon - 3000;
            ub = radiusMoon + 3000;
            step = 25;
            
            strIn = 'AS15_3_3_tiles.mat';
            mv = MultiViews(strIn);
            mv.raster([128 150]'); % ([64 64]');
            % mv.profile(lb,ub,step);
            % mv.profile_scx
            % mv.profile(lb,ub,step);
            % mv.pv.disp
        end
        
        function testProfile(self)
%             radiusMoon = 1737400;
%             lb = radiusMoon - 3000;
%             ub = radiusMoon + 3000;
%             step = 25;
%             
%             strIn = 'AS15_3_3_tiles.mat';
%             mv = MultiViews(strIn);
%             mv.raster([128 150]'); % ([64 64]');
%             % mv.profile(lb,ub,step);
%             % mv.profile_scx
%             % mv.profile(lb,ub,step);
%             % mv.pv.disp
        end
        
    end
end

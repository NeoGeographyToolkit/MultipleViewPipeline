classdef testFluxSearch < TestCase
    
    properties
    end
    
    methods
        function self = testFluxSearch(name)
            self = self@TestCase(name);
        end
        
        function setUp(self)
        end
        
        function tearDown(self)
            %            delete(self.mv);
        end
        
        function test1D(self)
            fprintf('Testing 1D\n');
            Q = eye(3); d = ones(3,1); b = rand(3,1); x0 = -b/2;
            options = optimset('Display','iter'); %,'TolFun',1e-2,'TolX',1e-2);
            
             x = lsqnonlin(@(x) sin(3*x),[1 4]);
            [x,f,exitflag,output] = fluxSearch(@(x)fcn(x),x0,Q,d,b,options)

            function f = fcn(x)
                f = x'*x;
            end
        end
    end
end
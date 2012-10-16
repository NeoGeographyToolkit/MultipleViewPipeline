classdef testLwsNonlin < TestCase
    
    properties
    end
    
    methods
        function self = testLwsNonlin(name)
            self = self@TestCase(name);
        end
        
        function setUp(self)
        end
        
        function tearDown(self)
            %            delete(self.mv);
        end
        
        function test1D(self)
            fprintf('Testing 1D\n');
            n = 1;
            Q = eye(n); d = ones(n,1); b = rand(n,1); x0 = -b/2;
            options = optimset('Display','iter'); %,'TolFun',1e-2,'TolX',1e-2);
            
            [x,f,exitflag,output] = lwsnonlin(@(x)fcn(x),x0,Q,d,b,options)

            function [f,g,fk,gk,wk] = fcn(x)
                c = 1e-5;
                f = x'*x+c*length(x);
                g = 2*x;
                fk = x.^2+c;
                gk = 2*x;
                wk = 1;
            end
        end
    end
end
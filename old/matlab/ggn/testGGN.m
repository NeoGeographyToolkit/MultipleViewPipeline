classdef testGGN < TestCase
    
    properties
    end
    
    methods
        function self = testGGN(name)
            self = self@TestCase(name);
        end
        
        function setUp(self)
        end
        
        function tearDown(self)
            %            delete(self.mv);
        end
        
        function testEgFunction(self)
            x = [1 1]'; rand(2,1);
            opts = optimset('GradObj','on','DerivativeCheck','on');
            fminunc(@egfun,x,opts)
        end
    end
end
function [x,f,exitflag,output] = ...
    fluxSearch(funfcn,x0,Q,d,b,options)
    n = length(d);
    [s,f,exitflag,output]=fminbnd(@(s)fcn(s),0,1,options);
    x = x0+(eye(n)-Q*diag(s.^d)*Q')*b;

    function f = fcn(s)
        x = x0+(eye(n)-Q*diag(s.^d)*Q')*b;
        f = funfcn(x);        
    end
end
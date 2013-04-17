function [X0] = ggn(FUN,X0,options,varargin)
[f,g,d,F,G] = feval(WFUN,X0);
c = zeros(length(d),1);
a = F.*(F-1);
for k = 1:10
    X = 
    c = G*(X0-X);
    a = offset(F0,F,c);
end
end

function a = offset(F0,F,c);
df = F-F0; cf = c.^2/4-F0.*F;
a = (cf+F)./(df-1);
idx = find(F+a<0 | isnan(df-1));
a(idx) = (cf(idx)-F(idx))./(df(idx)+1);
end
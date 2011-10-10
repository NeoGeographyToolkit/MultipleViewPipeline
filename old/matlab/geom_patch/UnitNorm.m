function [c,ceq,DC,DCeq] = UnitNorm(p)
% No nonlinear inequality constraints
c = [];
ceq = p(1:3)'*p(1:3)-1;

% Gradient of the constraints
if nargout > 2
    DC = [];
    DCeq = [2*p(1:3); 0];
end
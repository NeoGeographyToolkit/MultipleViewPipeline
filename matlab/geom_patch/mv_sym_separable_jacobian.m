clear all

syms sx sy cx cy real
syms v1 v2 v3 d real
v = [v1 v2 v3]'
e = [cx*cy sx*cy sy]';
de = [-sx*cy cx*cy 0; -cx*sy -sx*sy cy]';

for i=1:3
    for j=1:3
        for k=1:3
            str = sprintf('syms q%d%d%d real;', i,j,k);
            eval(str);
            str = sprintf('Q%d(%d,%d)=q%d%d%d;', i,j,k,i,j,k);
            eval(str);
        end
    end
end

for i=1:3
    for j=1:4
        str = sprintf('syms p%d%d real;',i,j);
        eval(str);
        str = sprintf('P(%d,%d)=p%d%d;',i,j,i,j);
        eval(str);
    end
    str = sprintf('p%d=transpose(P(%d,1:3));',i,i);
    eval(str);
end

u = d*P(:,1:3)*e+P(:,4)*v'*e

Q13 = u(3)*Q1-u(1)*Q3;
Q23 = u(3)*Q2-u(2)*Q3;
C = simple([v'*(u(3)*Q1-u(1)*Q3); v'*(u(3)*Q2-u(2)*Q3)]*de);
D = det(C);

B13 = [Q13 u(3)*p1-u(1)*p3];
B23 = [Q23 u(3)*p2-u(2)*p3];
B = [e'*B13; e'*B23];
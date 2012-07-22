syms a x y xz yz tx ty real

t = [tx ty]'
R = [cos(a) -sin(a); sin(a) cos(a)]
z = [x y]'
tz = [xz yz]'

E = z-R*(z+tz)-t
simple(expand(E'*E))

syms x y a b real
c = x*a+y*b;
s = x*b-y*a;
R = [c -s; s c];
D = [x y]*R*[a b]';
simple(D)

+ 2*x^2 + 2*y^2 
+ tx^2 + ty^2 
+ xz^2 + yz^2 
+ 2*x*xz 
- 2*ty*y 
- 2*tx*x 
+ 2*y*yz + 
- 2*x^2*cos(a) - 2*y^2*cos(a) 
+ 2*tx*x*cos(a) + 2*tx*xz*cos(a) 
+ 2*ty*y*cos(a) + 2*ty*yz*cos(a) 
- 2*x*xz*cos(a) - 2*y*yz*cos(a) 
+ 2*ty*x*sin(a) + 2*ty*xz*sin(a) 
- 2*tx*y*sin(a) - 2*tx*yz*sin(a) 
+ 2*x*yz*sin(a) - 2*xz*y*sin(a)

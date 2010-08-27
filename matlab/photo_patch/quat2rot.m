function R = quat2rot(q)
w2 = q(1)*q(1), x2 = q(2)*q(2), y2 = q(3)*q(3), z2 = q(4)*q(4);
wx = q(1)*q(2), wy = q(1)*q(3), wz = q(1)*q(4);
xy = q(2)*q(3), yz = q(3)*q(4), zx = q(4)*q(2);
R(1,1) = w2 + x2 - y2 - z2;
R(2,2) = w2 - x2 + y2 - z2;
R(3,3) = w2 - x2 - y2 + z2;
R(1,2) = 2 * (xy - wz);
R(1,3) = 2 * (zx + wy);
R(2,3) = 2 * (yz - wx);
R(2,1) = 2 * (xy + wz);
R(3,1) = 2 * (zx - wy);
R(3,2) = 2 * (yz + wx);
end
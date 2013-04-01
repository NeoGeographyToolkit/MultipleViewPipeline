function rot = quat2rot(quat)
  w = quat(1);
  x = quat(2);
  y = quat(3);
  z = quat(4);
  w2 = w*w;
  x2 = x*x;
  y2 = y*y;
  z2 = z*z;
  wx = w*x;
  wy = w*y;
  wz = w*z;
  xy = x*y;
  yz = y*z;
  zx = z*x;

  rot(1,1) = w2 + x2 - y2 - z2;
  rot(2,2) = w2 - x2 + y2 - z2;
  rot(3,3) = w2 - x2 - y2 + z2;
  rot(1,2) = 2 * (xy - wz);
  rot(1,3) = 2 * (zx + wy);
  rot(2,3) = 2 * (yz - wx);
  rot(2,1) = 2 * (xy + wz);
  rot(3,1) = 2 * (zx - wy);
  rot(3,2) = 2 * (yz + wx);

endfunction

%!test
%! quat = [5 6 8 10]' / 15;
%! rotref = [-0.457778 -0.017778 0.888889; 0.871111 -0.208889 0.444444; 0.177778 0.977778 0.111111];
%! assert(quat2rot(quat), rotref, 1e-6);

% vim:set syntax=octave:

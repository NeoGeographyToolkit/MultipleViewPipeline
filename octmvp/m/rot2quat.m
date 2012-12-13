% Copied from VW. TODO: WRITE TESTS!

function c = rot2quat(rot)
  d0 = rot(1,1); d1 = rot(2,2); d2 = rot(3,3);
  ww = 1.0 + d0 + d1 + d2;
  xx = 1.0 + d0 - d1 - d2;
  yy = 1.0 - d0 + d1 - d2;
  zz = 1.0 - d0 - d1 + d2;

  max_entry = ww;
  if (xx > max_entry) 
    max_entry = xx;
  endif
  if (yy > max_entry) 
    max_entry = yy;
  endif
  if (zz > max_entry) 
    max_entry = zz;
  endif

  c = zeros(4, 1);
  if (ww == max_entry)
    w4 = sqrt(ww * 4.0);
    c(1) = w4 / 4;
    c(2) = (rot(3,2) - rot(2,3)) / w4;
    c(3) = (rot(1,3) - rot(3,1)) / w4;
    c(4) = (rot(2,1) - rot(1,2)) / w4;
  elseif (xx == max_entry)
    x4 = sqrt(xx * 4.0);
    c(1) = (rot(3,2) - rot(2,3)) / x4;
    c(2) = x4 / 4;
    c(3) = (rot(1,2) + rot(2,1)) / x4;
    c(4) = (rot(1,3) + rot(3,1)) / x4;
  elseif (yy == max_entry)
    y4 = sqrt(yy * 4.0);
    c(1) = (rot(1,3) - rot(3,1)) / y4;
    c(2) = (rot(1,2) + rot(2,1)) / y4;
    c(3) =  y4 / 4;
    c(4) = (rot(2,3) + rot(3,2)) / y4;
  else
    z4 = sqrt(zz * 4.0);
    c(1) = (rot(2,1) - rot(1,2)) / z4;
    c(2) = (rot(1,3) + rot(3,1)) / z4;
    c(3) = (rot(2,3) + rot(3,2)) / z4;
    c(4) =  z4 / 4;
  endif

endfunction

%!test
%! rot = [-0.457778 -0.017778 0.888889; 0.871111 -0.208889 0.444444; 0.177778 0.977778 0.111111];
%!
%! quat = rot2quat(rot);
%! quatref = [5 6 8 10]' / 15;
%! assert(quat, quatref, 1e-6);

% vim:set syntax=octave:

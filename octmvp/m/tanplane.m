function q = tanplane(e)
  e /= norm(e);
  ex = [-e(2) e(1) 0]';
  ex /= norm(ex);
  ey = -cross(e, ex);
  R = [ex ey -e];
  q = rot2quat(R);
endfunction

% vim:set syntax=octave:

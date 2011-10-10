% mesh is a mesh where a typical vertex is (X(r,c), Y(r,c), Z(r,c))
% orbits has a img and cam
function [xw yw] = _orthoproj_impl(xx, yy, zz, orbit)
  dim = size(xx);

  D = [xx(:), yy(:), zz(:), ones(dim(1) * dim(2), 1)]';
  PD = orbit.cam * D;
  xw = reshape(PD(1,:) ./ PD(3,:), dim);
  yw = reshape(PD(2,:) ./ PD(3,:), dim);

endfunction

% vim:set syntax=octave:

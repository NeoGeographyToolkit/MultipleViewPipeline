% mesh is a mesh where a typical vertex is (X(r,c), Y(r,c), Z(r,c))
% orbits has a img and cam
function [xw yw] = _orthoproj_impl(xx, yy, zz, orbit)
  dim = size(xx);

  xw = zeros(dim);
  yw = zeros(dim);

  for r = 1:dim(1)
    for c = 1:dim(2)
      % pt -> px
      px = orbit.cam * [xx(r,c); yy(r,c); zz(r,c); 1];

      % store in map (and normalize homog coords)
      xw(r, c) = px(1) / px(3);
      yw(r, c) = px(2) / px(3);
    endfor
  endfor
endfunction

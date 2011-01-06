% mesh is a mesh where a typical vertex is (X(r,c), Y(r,c), Z(r,c))
% orbits has a img and cam
function patch = orthoproj(xx, yy, zz, orbit)
  if ((any(size(xx) != size(yy))) || (any(size(yy) != size(zz))))
    error("xx yy and zz must have same dim");
  endif

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

  patch = imremap(orbit.img, xw, yw, "bilinear");

endfunction

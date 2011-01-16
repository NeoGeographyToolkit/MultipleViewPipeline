% mesh is a mesh where a typical vertex is (X(r,c), Y(r,c), Z(r,c))
% orbits has a img and cam
function patch = orthoproj(xx, yy, zz, orbit)
  if ((any(size(xx) != size(yy))) || (any(size(yy) != size(zz))))
    error("xx yy and zz must have same dim");
  endif

  [xw yw] = _orthoproj_impl(xx, yy, zz, orbit);

  patch = imremap(orbit.img, xw, yw, "bilinear");

endfunction

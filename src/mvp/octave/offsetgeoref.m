function result = offsetgeoref(georef, offset)
  offmat = eye(3);
  offmat(1:2, 3) = offset' - 1;

  result = georef;
  result.transform = result.transform * offmat;
endfunction

%!test
%! georef.transform = [0.00002,  0.00000, 0.98145;
%!                     0.00000, -0.00002, 0.16900;
%!                     0.00000,  0.00000, 1.00000];
%! offgeo = offsetgeoref(georef, [100 200]);
%! pt = georef.transform * [100; 200; 1];
%! offpt = offgeo.transform * [1; 1; 1];
%! assert(pt, offpt, 1e-6)

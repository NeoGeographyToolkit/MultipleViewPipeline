function [xw, yw] = _porthoproj_impl(camera, planeNormal, planeD, georef, hWin)
  dim = fliplr(hWin * 2 + 1);

  cntrPtLonLatH = georef.transform * ones(3, 1);
  cntrPtLonLat = cntrPtLonLatH(1:2) / cntrPtLonLatH(3);
  [e de_d0] = lonlat2normal(cntrPtLonLat);

  deltaE = e - de_d0 * cntrPtLonLat;

  P = camera * [eye(3,3);planeNormal'/planeD] * [de_d0 deltaE] * georef.transform;

  [X, Y] = meshgrid((-hWin(1):hWin(1))+1, (-hWin(2):hWin(2))+1);
  D = [X(:), Y(:), ones(prod(dim), 1)]';
  PD = P * D;

  xw = reshape(PD(1,:) ./ PD(3,:), dim);
  yw = reshape(PD(2,:) ./ PD(3,:), dim);
endfunction

%!test
%! planeNormal = [0.54543; 0.82112; 0.16812];
%! planeD = 1.7348e6;
%! georef.transform = [0.00002,  0.00000, 0.98145;
%!                     0.00000, -0.00002, 0.16900;
%!                     0.00000,  0.00000, 1.00000];
%! georef.datum.semi_major_axis = 1737400;
%! georef.datum.semi_minor_axis = 1737400;
%! camera = [-3.9162e+03,  3.0436e+02, -1.8351e+03, 3.8275e+09;
%!            1.5020e+03, -5.3986e+02, -3.4544e+03, 3.6555e+08;
%!           -5.3071e-01, -8.3578e-01, -1.4081e-01, 1.8546e+06];
%! [xw yw] = _porthoproj_impl(camera, planeNormal, planeD, georef, [7 8]);
%! [xw1 yw1] = _porthoproj_impl_ref(camera, planeNormal, planeD, georef, [7 8]);
%! errX = norm(xw1(:) - xw(:));
%! errY = norm(yw1(:) - yw(:));
%! assert(errX, 0, 0.01);
%! assert(errY, 0, 0.01);

% vim:set syntax=octave:

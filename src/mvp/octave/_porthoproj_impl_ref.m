function [xw, yw] = _porthoproj_impl_ref(camera, planeNormal, planeD, georef, hWin)
  if (isscalar(hWin))
    hWin = [hWin hWin];
  endif

  dim = hWin * 2 + 1;

  xw = zeros(fliplr(dim));
  yw = zeros(fliplr(dim));

  for r = 1:dim(2)
    for c = 2:dim(1)
      px = c - hWin(1);
      py = r - hWin(2);

      lonlat_H = georef.transform * [px; py; 1];
      lonlat = lonlat_H(1:2) / lonlat_H(3);

      % TODO: Make this calc work for non-spheroid datums
      radius = planeD / dot(planeNormal, lonlat2normal(lonlat));
      alt = radius - georef.datum.semi_major_axis;

      xyz_H = [lonlatalt2xyz(georef.datum, lonlat, alt); 1];

      % The point in the orbital image      
      op_H = camera * xyz_H;

      xw(r, c) = op_H(1) / op_H(3);
      yw(r, c) = op_H(2) / op_H(3);
    endfor
  endfor
endfunction

%TODO: Make a real test?
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
%! [xw, yw] = _porthoproj_impl_ref(camera, planeNormal, planeD, georef, 5);

% vim:set syntax=octave:

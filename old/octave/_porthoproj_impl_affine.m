function [xw yw] = _porthoproj_impl_affine(planeNormal, planeD, demPt, georef, orbit, hWin)
  % This function assumes georef is affine
  % Doesn't work well for large values of demPt?
  dim = 2 * hWin + 1;

  cntrPtLonLatH = georef * [demPt(1); demPt(2); 1];
  [e de_d0] = lonlat2normal(cntrPtLonLatH(1:2) / cntrPtLonLatH(3));

  H1 = orbit.cam * [eye(3,3);planeNormal'/planeD];
  dH1_de = (H1(1:2,:) - [H1(1,:)*e;H1(2,:)*e]*H1(3,:)) / (H1(3,:) * e);
  S = dH1_de * de_d0 * georef(1:2, 1:2);
  
  cntrPtOrbH = H1 * e;
  cntrPtOrb = cntrPtOrbH(1:2) / cntrPtOrbH(3);
  offset = cntrPtOrb - S * [demPt(1); demPt(2)];

  P = [[S, offset]; [0, 0, 1]];

  [X, Y] = meshgrid((0:dim - 1) + (demPt(1) - hWin), (0:dim - 1) + (demPt(2) - hWin));
  D = [X(:), Y(:), ones(dim * dim, 1)]';
  PD = P * D;
  xw = reshape(PD(1,:), dim, dim); % Don't need ./ PD(3,:) because is affine
  yw = reshape(PD(2,:), dim, dim); % Don't need ./ PD(3,:) because is affine
endfunction


%!test
%! planeNormal = [0.54543; 0.82112; 0.16812];
%! planeD = 1.7348e6;
%! demPt = [100; 200];
%! georef = [0.00002,  0.00000, 0.98145;
%!           0.00000, -0.00002, 0.16900;
%!           0.00000,  0.00000, 1.00000];
%! orbit.cam = [-3.9162e+03,  3.0436e+02, -1.8351e+03, 3.8275e+09;
%!               1.5020e+03, -5.3986e+02, -3.4544e+03, 3.6555e+08;
%!              -5.3071e-01, -8.3578e-01, -1.4081e-01, 1.8546e+06];
%! [xw yw] = _porthoproj_impl_affine(planeNormal, planeD, demPt, georef, orbit, 7);
%! [xw1 yw1] = _porthoproj_impl_ref(planeNormal, planeD, demPt, georef, orbit, 7);
%! errX = norm(xw1(:) - xw(:));
%! errY = norm(yw1(:) - yw(:));
%! assert(errX, 0, 0.4);
%! assert(errY, 0, 0.4);

% vim:set syntax=octave:

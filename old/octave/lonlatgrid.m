function [lon lat] = lonlatgrid(georef, sz, off = [1 1])
  if (isscalar(sz))
    sz = [sz sz];
  endif

  [X, Y] = meshgrid((0:sz(2) - 1) + off(1), (0:sz(1) - 1) + off(2));
  D = [X(:), Y(:), ones(sz(1) * sz(2), 1)]';
  PD = georef * D;

  lon = reshape(PD(1,:) ./ PD(3,:), sz);
  lat = reshape(PD(2,:) ./ PD(3,:), sz);
endfunction

%!test
%! sz = [4, 3];
%! off = [5, 3];
%! georef = [0.00002,  0.00000, 0.98145;
%!           0.00000, -0.00002, 0.16900;
%!           0.00030,  0.00200, 1.00000];
%! [lon_imp lat_imp] = lonlatgrid(georef, sz, off);
%! for r = 1:sz(1)
%!   for c = 1:sz(2)
%!     % px -> lonlat
%!     ll = georef * [c + off(1) - 1;r + off(2) - 1;1];
%!
%!     % store in result (and normalize homog coords)
%!     lon(r, c) = ll(1) / ll(3);
%!     lat(r, c) = ll(2) / ll(3);
%!   endfor
%! endfor
%! errLon = norm(lon_imp(:) - lon(:));
%! errLat = norm(lat_imp(:) - lat(:));
%! assert(errLon, 0, 1e-6);
%! assert(errLat, 0, 1e-6);

% vim:set syntax=octave:

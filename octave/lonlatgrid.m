function [lon lat] = lonlatgrid(georef, sz, off = [1 1])
  if (isscalar(sz))
    sz = [sz sz];
  endif

  lon = zeros(sz);
  lat = zeros(sz);

  for r = 1:sz(1)
    for c = 1:sz(2)
      % px -> lonlat
      ll = georef * [c + off(1) - 1;r + off(2) - 1;1];

      % store in result (and normalize homog coords)
      lon(r, c) = ll(1) / ll(3);
      lat(r, c) = ll(2) / ll(3);
    endfor
  endfor    
endfunction

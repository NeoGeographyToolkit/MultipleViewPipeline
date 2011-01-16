function [lon lat] = lonlatgrid(georef, sz, off = [1 1])
  lon = zeros(sz);
  lat = zeros(sz);
  for r = off(1):sz(1)
    for c = off(2):sz(2)
      % px -> lonlat
      ll = georef * [c;r;1];

      % store in result (and normalize homog coords)
      lon(r, c) = ll(1) / ll(3);
      lat(r, c) = ll(2) / ll(3);
    endfor
  endfor    
endfunction

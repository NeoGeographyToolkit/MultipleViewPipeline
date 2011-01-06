function [lon lat] = lonlatgrid(georef, sz)
  lon = zeros(sz);
  lat = zeros(sz);
  for r = 1:sz(1)
    for c = 1:sz(2)
      % px -> lonlat
      ll = georef * [c;r;1];

      % store in result (and normalize homog coords)
      lon(r, c) = ll(1) / ll(3);
      lat(r, c) = ll(2) / ll(3);
    endfor
  endfor    
endfunction

function rad = planedem(normal, d, lon, lat)
  if (any(size(lon) != size(lat)))
    error("lon and lat dims must be the same");
  endif

  dim = size(lon);
  
  rad = zeros(dim);

  for r = 1:dim(1)
    for c = 1:dim(2)
      rad(r, c) = d / dot(normal, lonlat2normal([lon(r, c) lat(r, c)]));
    endfor
  endfor 
endfunction

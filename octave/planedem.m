function rad = planedem(plane, lon, lat)
  if (any(size(lon) != size(lat)))
    error("lon and lat dims must be the same");
  endif
  if (any(size(plane) != [1 4]))
    error("Valid planes must have size() = [1 4]");
  endif

  dim = size(lon);
  
  rad = zeros(dim);

  for r = 1:dim(1)
    for c = 1:dim(2)
      rad(r, c) = plane(4) / dot(plane(1:3), lonlat2normal([lon(r, c) lat(r, c)]));
    endfor
  endfor 
endfunction

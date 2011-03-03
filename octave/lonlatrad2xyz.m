function [xx yy zz] = lonlatrad2xyz(lon, lat, rad)
  if ((any(size(lon) != size(lat))) || (any(size(lat) != size(rad))))
    error("lon, lat and rad must have same dim");
  endif

  dim = size(lon);

  for r = 1:dim(1)
    for c = 1:dim(2)
      xyz = rad(r,c) * lonlat2normal([lon(r,c); lat(r,c)]);

      xx(r,c) = xyz(1);
      yy(r,c) = xyz(2);
      zz(r,c) = xyz(3);
    endfor
  endfor

endfunction

% vim:set syntax=octave:

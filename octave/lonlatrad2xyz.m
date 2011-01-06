function [xx yy zz] = lonlatrad2xyz(lon, lat, rad)
  if ((any(size(lon) != size(lat))) || (any(size(lat) != size(rad))))
    error("lon, lat and rad must have same dim");
  endif

  dim = size(lon);

  for r = 1:dim(1)
    for c = 1:dim(2)
      % east positive
      normxy = rad(r,c) * cos(lat(r,c));

      xx(r,c) = normxy * cos(lon(r,c));
      yy(r,c) = normxy * sin(lon(r,c));
      zz(r,c) = rad(r,c) * sin(lat(r,c));
    endfor
  endfor

endfunction

function [result, variance, converged, num_iterations] = mvpfootprint(seed, georef, images, settings)
  lonlat_h = georef.transform * [1; 1; 1];
  lonlat = lonlat_h(1:2) / lonlat_h(3);

  xyz = lonlatalt2xyz(georef.datum, lonlat, 0);
  xyz_h = [xyz; 1];

  overlap = 0;

  for img = images
    px_h = img.camera * xyz_h;
    px = px_h(1:2) /  px_h(3);

    if (px >= [1; 1] && px <= flipud(size(img.data)'))
      overlap++;
    endif
  endfor

  result.alt = overlap;
  result.orientation = [overlap; overlap; overlap];
  result.windows = [overlap; overlap; overlap];
  variance = overlap;
  converged = overlap > 0;
  num_iterations = overlap;
endfunction

% vim:set syntax=octave:

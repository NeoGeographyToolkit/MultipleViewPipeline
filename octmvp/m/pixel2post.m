function post = pixel2post(georef, pixel)
  lonlat = georef.pixel_to_lonlat(pixel);
  post = georef.datum().geodetic_to_cartesian([lonlat;0]);
  post /= norm(post);
endfunction

% vim:set syntax=octave:

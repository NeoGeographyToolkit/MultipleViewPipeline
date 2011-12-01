function patches = mvppatches(images, georef, alt, planeNormal, windows)
  lonlat_H = georef.transform * ones(3, 1);
  lonlat = lonlat_H(1:2) / lonlat_H(3);
  xyz = lonlatalt2xyz(georef.datum, lonlat, alt);

  planeD = dot(xyz, planeNormal);

  patches = arrayfun(@(i) porthoproj(i, planeNormal, planeD, georef, windows(1:2)), images, "UniformOutput", false);

endfunction

% vim:set syntax=octave:

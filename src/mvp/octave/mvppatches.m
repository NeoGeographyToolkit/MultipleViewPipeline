function patches = mvppatches(images, georef, alt, orientation, windows)
  lonlat_h = georef.transform * ones(3, 1);
  lonlat = lonlat_h(1:2) / lonlat_h(3);
  xyz = lonlatalt2xyz(georef.datum, lonlat, alt);

  planeNormal = orientation;
  planeD = dot(xyz, planeNormal);

  patches = arrayfun(@(i) porthoproj(i, planeNormal, planeD, georef, windows(1:2)), images, "UniformOutput", false);
endfunction

% vim:set syntax=octave:

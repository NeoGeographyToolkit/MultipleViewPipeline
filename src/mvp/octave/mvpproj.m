function projs = mvpproj(alt, orientation, windows, georef, images)
  lonlat_h = georef.transform * ones(3, 1);
  lonlat = lonlat_h(1:2) / lonlat_h(3);
  xyz = lonlatalt2xyz(georef.datum, lonlat, alt);

  planeNormal = orientation;
  planeD = dot(xyz, planeNormal);
  projSize = mvppatches("projsize", windows);

  projs = arrayfun(@(i) porthoproj(i, planeNormal, planeD, georef, projSize), images, "UniformOutput", false);
endfunction

% vim:set syntax=octave:

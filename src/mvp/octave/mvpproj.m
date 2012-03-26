function projs = mvpproj(alt, orientation, windows, georef, images, mvpoptions)
  windows_px = windows * mvpoptions.gauss_divisor;
  projSize = windows_px(1:2) + (windows_px(3) - 1);

  lonlat_h = georef.transform * ones(3, 1);
  lonlat = lonlat_h(1:2) / lonlat_h(3);
  xyz = lonlatalt2xyz(georef.datum, lonlat, alt);

  planeNormal = orientation;
  planeD = dot(xyz, planeNormal);

  projs = arrayfun(@(i) porthoproj(i, planeNormal, planeD, georef, projSize), images, "UniformOutput", false);
endfunction

% vim:set syntax=octave:

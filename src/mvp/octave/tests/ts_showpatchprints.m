function ts_showpatchprints(seed, patch_georef, images)
  planeNormal = seed.orientation;
  win = gausskernel("kernsize", seed.windows(1:2));

  lonlat_H = patch_georef.transform * ones(3, 1);
  lonlat = lonlat_H(1:2) / lonlat_H(3);
  xyz = lonlatalt2xyz(patch_georef.datum, lonlat, seed.alt);

  planeD = dot(xyz, planeNormal);

  numImages = numel(images);

  gridWidth = ceil(sqrt(numImages));
  gridHeight = ceil(numImages / gridWidth);

  for currImage = 1:numImages
    subplot(gridHeight, gridWidth, currImage);
    imagesc(images(currImage).data);
    hold on;
    [xw yw] = _porthoproj_impl(images(currImage).camera, planeNormal, planeD, patch_georef, win);
    hullIdx = convhull(xw(:), yw(:), "Pp");
    fill(xw(hullIdx), yw(hullIdx), 'w');
    hold off;
    currImage++;
  endfor
endfunction

% vim:set syntax=octave:

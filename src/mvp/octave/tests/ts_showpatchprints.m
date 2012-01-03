function ts_showpatchprints(seed, patch_georef, images)
  planeNormal = seed.orientation;
  hWin = seed.windows(1:2);

  lonlat_H = patch_georef.transform * ones(3, 1);
  lonlat = lonlat_H(1:2) / lonlat_H(3);
  xyz = lonlatalt2xyz(patch_georef.datum, lonlat, seed.alt);

  planeD = dot(xyz, planeNormal);

  numImages = numel(images);

  currImage = 1;
  while (currImage != -1)
    printf("Current image: %d\n\n", currImage);
    fflush(stdout);

    imagesc(images(currImage).data);
    hold on;
    [xw yw] = _porthoproj_impl(images(currImage).camera, planeNormal, planeD, patch_georef, hWin);
    hullIdx = convhull(xw(:), yw(:), "Pp");
    fill(xw(hullIdx), yw(hullIdx), 'w');
    hold off

    [locX locY btn] = ginput(1);
    if (btn == 1)
      currImage += 1;
    elseif (btn == 2)
      currImage -= 1;
    else
      break;
    endif
    currImage = mod(currImage - 1, numImages) + 1;
  endwhile
endfunction

% vim:set syntax=octave:

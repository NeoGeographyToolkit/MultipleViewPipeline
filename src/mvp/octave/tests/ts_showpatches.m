function ts_showpatches(seed, patch_georef, images)
  patches = mvppatches(images, patch_georef, seed.alt, seed.orientation, seed.windows);

  numPatches = numel(patches);

  currPatch = 1;
  while (currPatch != -1)
    printf("Current patch: %d\n\n", currPatch);
    imagesc(patches{currPatch});
    fflush(stdout);
    [locX locY btn] = ginput(1);
    if (btn == 1)
      currPatch += 1;
    elseif (btn == 2)
      currPatch -= 1;
    else
      break;
    endif
    currPatch = mod(currPatch - 1, numPatches) + 1;
  endwhile
endfunction

% vim:set syntax=octave:

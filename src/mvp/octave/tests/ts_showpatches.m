function ts_showpatches(seed, georef, images)
  patches = mvpproj(seed.alt, seed.orientation, seed.windows, georef, images);

  numPatches = numel(patches);

  currPatch = 1;
  while (true)
    printf("Current patch: %d\n\n", currPatch);
    imagesc(patches{currPatch});
    fflush(stdout);
    [locX locY btn] = ginput(1);
    if (btn == 1)
      currPatch += 1;
    elseif (btn == 3)
      currPatch -= 1;
    else
      break;
    endif
    currPatch = mod(currPatch - 1, numPatches) + 1;
  endwhile
endfunction

% vim:set syntax=octave:

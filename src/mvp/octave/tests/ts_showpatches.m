function ts_showpatches(seed, georef, images)
  patches = mvpproj(seed.alt, seed.orientation, seed.windows, georef, images);

  numPatches = numel(patches);

  gridWidth = ceil(sqrt(numPatches));
  gridHeight = ceil(numPatches / gridWidth);

  for currPatch = 1:numPatches
    if (currPatch > numPatches)
      break;
    endif 

    subplot(gridHeight, gridWidth, currPatch);
    imagesc(patches{currPatch});
    title(["Patch " num2str(currPatch)]);
    axis("square");
    currPatch++;
  endfor
endfunction

% vim:set syntax=octave:

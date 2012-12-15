function patches = weight_patches(raw_patches, gwindow, smooth, gsmooth)
  for i = 1:numel(raw_patches)
    idx = isna(raw_patches{i});
    raw_patches{i}(:, :, 2) = !idx;
    raw_patches{i}(idx) = 0;
    % TODO: Weight with gaussian window
    % TODO: Blur patch with gaussian blur
  endfor

  patches = raw_patches;

endfunction

% vim:set syntax=octave:

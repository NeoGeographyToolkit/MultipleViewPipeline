function obj = mvpobj(alt, orientation, windows, georef, images, mvpoptions)
  projs = mvpproj(alt, orientation, windows, georef, images);

  if (numel(projs) < 2)
    obj = NA;
    return;
  endif

  [patches weights] = mvppatches(projs, windows);

  if (size(patches)(3) < 2)
    obj = NA;
    return;
  endif

  if (mvpoptions.alt_range == -1)
    obj = mvprefl(patches, weights);
  else
    obj = mvprefl_fast(patches, weights);
  endif
endfunction

% vim:set syntax=octave:

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

  if (mvpoptions.fast_reflectance)
    obj = mvprefl_fast(patches, weights);
  else
    obj = mvprefl(patches, weights);
  endif
endfunction

% vim:set syntax=octave:

function variance = _mvpalgorithm_windows(alt, orientation, windows, georef, images, settings)
  global gResult;

  variance = mvpobj(alt, orientation, windows, georef, images, settings);

  if (variance < gVariance)
    gVariance = variance;
    gResult.alt = alt;
    gResult.orientation = orientation;
    gResult.windows = windows;
  endif

endfunction

% vim:set syntax=octave:

function variance = _mvpalgorithm_windows(alt, orientation, windows, georef, images, mvpoptions)
  global gResult;

  variance = mvpobj(alt, orientation, windows, georef, images, mvpoptions);

  if (variance < gVariance)
    gVariance = variance;
    gResult.alt = alt;
    gResult.orientation = orientation;
    gResult.windows = windows;
  endif

endfunction

% vim:set syntax=octave:

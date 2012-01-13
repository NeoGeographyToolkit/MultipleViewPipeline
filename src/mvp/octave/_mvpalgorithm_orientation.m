
function variance = _mvpalgorithm_orientation(alt, orientation, georef, images, settings)
  global gResult;

  if (settings.fix_windows)
    variance = _mvpalgorithm_windows(alt, orientation, gResult.windows, georef, images, settings);
  else
    error("Unfixed orientation not implemented yet");
    % [windows variance] = fmincon(@(w) _mvpalgorithm_windows(alt, orientation, w, georef, images, settings), gResult.windows);
  endif
endfunction

% vim:set syntax=octave:

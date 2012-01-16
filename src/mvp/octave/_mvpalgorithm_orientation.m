
function variance = _mvpalgorithm_orientation(alt, orientation, georef, images, mvpoptions)
  global gResult;

  if (mvpoptions.fix_windows)
    variance = _mvpalgorithm_windows(alt, orientation, gResult.windows, georef, images, mvpoptions);
  else
    error("Unfixed orientation not implemented yet");
    % [windows variance] = fmincon(@(w) _mvpalgorithm_windows(alt, orientation, w, georef, images, mvpoptions), gResult.windows);
  endif
endfunction

% vim:set syntax=octave:

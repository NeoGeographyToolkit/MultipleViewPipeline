
% 
function variance = _mvpalgorithm_alt(alt, georef, images, settings)
  global gResult;

  if (settings.fix_orientation)
    variance = _mvpalgorithm_orientation(alt, gResult.orientation, georef, images, settings); 
  else
    error("Unfixed orientation not implemented yet");
    % [orientation variance ] = fmincon(@(o) _mvpalgorithm_orientation(alt, o, georef, images, settings), gResult.orientation);
  endif
endfunction

% vim:set syntax=octave:

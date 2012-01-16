
% 
function variance = _mvpalgorithm_alt(alt, georef, images, mvpoptions)
  global gResult;

  if (mvpoptions.fix_orientation)
    variance = _mvpalgorithm_orientation(alt, gResult.orientation, georef, images, mvpoptions); 
  else
    error("Unfixed orientation not implemented yet");
    % [orientation variance ] = fmincon(@(o) _mvpalgorithm_orientation(alt, o, georef, images, mvpoptions), gResult.orientation);
  endif
endfunction

% vim:set syntax=octave:

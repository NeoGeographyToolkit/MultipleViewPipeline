% Convention: everything is [x y] until it is indexted in a matrix: M(y, x)
  
function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, settings)
  % Initialize the result
  global gResult;
  gResult = seed;
  
  global gVariance;
  gVariance = Inf;

  opts = optimset("MaxIter", settings.max_iterations, "FunValCheck", "on");

  try
    if (settings.fix_orientation && settings.fix_windows)
      [gResult.alt gVariance info output] = fminbnd(@(a) mvpobj(a, gResult.orientation, gResult.windows, georef, images, settings), 
                                                    settings.alt_min, settings.alt_max, opts);
    else
      [alt variance info output] = fminbnd(@(a) _mvpalgorithm_alt(a, georef, images, settings),
                                           settings.alt_min, settings.alt_max, opts);
    endif

    result = gResult;
    variance = gVariance;
    converged = (info == 1);
    num_iterations = output.iterations;
  catch
    result.alt = NA;
    result.orientation = NA;
    result.windows = NA;
    variance = NA;
    converged = false;
    num_iterations = 0;
  end_try_catch

endfunction

% vim:set syntax=octave:

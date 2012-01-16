% Convention: everything is [x y] until it is indexted in a matrix: M(y, x)
  
function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, mvpoptions)
  % Initialize the result
  global gResult;
  gResult = seed;
  
  global gVariance;
  gVariance = Inf;

  opts = optimset("MaxIter", mvpoptions.max_iterations, "FunValCheck", "on");

  try
    if (mvpoptions.fix_orientation && mvpoptions.fix_windows)
      [gResult.alt gVariance info output] = fminbnd(@(a) mvpobj(a, gResult.orientation, gResult.windows, georef, images, mvpoptions), 
                                                                mvpoptions.alt_min, mvpoptions.alt_max, opts);
    else
      [alt variance info output] = fminbnd(@(a) _mvpalgorithm_alt(a, georef, images, mvpoptions),
                                                                  mvpoptions.alt_min, mvpoptions.alt_max, opts);
    endif

    result = gResult;
    variance = gVariance;
    converged = (info == 1);
    num_iterations = output.iterations;
  catch
    result.alt = NA;
    result.orientation = [NA NA NA];
    result.windows = [NA NA NA];
    variance = NA;
    converged = false;
    num_iterations = 0;
  end_try_catch

endfunction

% vim:set syntax=octave:

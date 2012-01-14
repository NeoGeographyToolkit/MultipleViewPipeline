% Convention: everything is [x y] until it is indexted in a matrix: M(y, x)
  
function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, settings)
  opts = optimset("MaxIter", settings.max_iterations, "FunValCheck", "on");

  try
    [result.alt variance info output] = fminbnd(@(a) mvpobj(images, georef, a, orientation, windows), 
                                                            settings.alt_min, settings.alt_max, opts);
    result.orientation = orientation;
    result.windows = windows;

    converged = (info == 1);
    num_iterations = output.iterations;
  catch
    result.alt = NA;
    variance = NA;
    info = 0;

    result.orientation = orientation;
    result.windows = windows;

    converged = false;
    num_iterations = 0;
  end_try_catch

endfunction

% vim:set syntax=octave:

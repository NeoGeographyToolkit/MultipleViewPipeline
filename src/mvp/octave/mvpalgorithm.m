% Convention: everything is [x y] until it is indexted in a matrix: M(y, x)
  
function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, mvpoptions)
  result = seed;

  try
    if (mvpoptions.alt_range > 0)
      opts = optimset("MaxIter", mvpoptions.max_iterations, "FunValCheck", "on");
      opts = optimset(opts, "TolX", 1e-2);

      altMin = seed.alt - mvpoptions.alt_range;
      altMax = seed.alt + mvpoptions.alt_range;

      [result.alt variance info output] = fminbnd(@(a) mvpobj(a, seed.orientation, seed.windows, georef, images, mvpoptions), 
                                                              altMin, altMax, opts);
      converged = (info == 1);
      num_iterations = output.iterations;
    else
      opts = optimset("MaxIter", mvpoptions.max_iterations, "FunValCheck", "on");
      opts = optimset(opts, "TolX", 1e-8, "TolFun", 1e-8);

      [result.alt variance info output] = fminunc(@(a) mvpobj(a, seed.orientation, seed.windows, georef, images, mvpoptions), seed.alt, opts);

      converged = (info > 0);
      num_iterations = output.iterations;
    endif

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

% Convention: everything is [x y] until it is indexted in a matrix: M(y, x)
  
function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, mvpoptions)
  % Initialize the result
  global gResult;
  gResult = seed;
  
  global gVariance;
  gVariance = Inf;

  try
    if (mvpoptions.alt_range > 0)
      opts = optimset("MaxIter", mvpoptions.max_iterations, "FunValCheck", "on", "TolX", mvpoptions.alt_tolerance);

      altMin = seed.alt - mvpoptions.alt_range;
      altMax = seed.alt + mvpoptions.alt_range;

      [gResult.alt gVariance info output] = fminbnd(@(a) mvpobj(a, gResult.orientation, gResult.windows, georef, images, mvpoptions), 
                                                                altMin, altMax, opts);
      result = gResult;
      variance = gVariance;
      converged = (info == 1);
      num_iterations = output.iterations;
    else
      opts = optimset("MaxIter", mvpoptions.max_iterations, "FunValCheck", "on");
      opts = optimset(opts, "TolX", 1e-8, "TolFun", 1e-8);

      [gResult.alt gVariance info output] = fminunc(@(a) mvpobj(a, gResult.orientation, gResult.windows, georef, images, mvpoptions), seed.alt, opts);
      result = gResult;
      variance = gVariance;
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

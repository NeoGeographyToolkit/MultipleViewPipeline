% Convention: everything is [x y] until it is indexted in a matrix: M(y, x)
  
function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, settings)
  if (settings.test_algorithm)
    [result, variance, converged, num_iterations] = mvptestalgorithm(seed, georef, images, settings);
    return;
  endif

  % Choose a orientation normal to the planet's surface
  lonlatPt_H = georef.transform * ones(3, 1);
  lonlatPt = lonlatPt_H(1:2) ./ lonlatPt_H(3);
  orientation = lonlat2normal(lonlatPt);

  % TODO: Windows should actually come from the seed
  windows = [10; 10; 1];

  opts = optimset("MaxIter", 60, "FunValCheck", "on");

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

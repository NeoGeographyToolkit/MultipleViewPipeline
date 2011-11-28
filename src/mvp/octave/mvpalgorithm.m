function [result, variance, converged, num_iterations] = mvpalgorithm(seed, georef, images, settings)
  if (settings.test_algorithm)
    [result, variance, converged, num_iterations] = mvptestalgorithm(seed, georef, images, settings);
    return;
  endif

  result = seed;
  variance = 0;
  converged = 1;
  num_iterations = 0;

endfunction

function options = mvpoptimset(alt_range)
  options.alt_range = alt_range;
  options.fix_orientation = true;
  options.fix_windows = true;
  options.max_iterations = 80;
  options.gauss_divisor = 6;
  options.fast_reflectance = true;
endfunction


% vim:set syntax=octave:

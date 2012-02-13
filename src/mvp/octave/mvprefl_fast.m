function obj = mvprefl_fast(patches, weights)
  prevWarnState = warning("query", "Octave:divide-by-zero");
  warning("off", "Octave:divide-by-zero");

  sz = size(patches);
  dim = sz(1:2);
  numPatches = sz(3);

  weightSums = sum(sum(weights));

  means = sum(sum(patches .* weights)) ./ weightSums;
  
  zmPatches = patches - repmat(means, [dim 1]);

  sqDevs = zmPatches .^ 2;

  stddevs = sqrt(sum(sum(sqDevs .* weights)) ./ weightSums);

  normPatches = zmPatches ./ repmat(stddevs, [dim 1]);

  % Catch divide by zero
  normPatches(find(!isfinite(normPatches))) = 0;

  albedo = sum(normPatches .* weights, 3) ./ sum(weights, 3);

  % Catch divide by zero
  albedo(find(!isfinite(albedo))) = 0;

  diffs = abs(normPatches - repmat(albedo, [1 1 numPatches])) .* weights;

  obj = sum(diffs(:)) / sum(weightSums(:));

  warning(prevWarnState.state, prevWarnState.identifier);
endfunction

%!test
%! _mvprefl_testhelpers
%! [patches weights a b] = genpatches();
%! e = mvprefl_fast(patches, weights);
%! e2 = mvprefl_fast_ref(patches, weights);
%! assert(e, e2, 1e-8);

% Test divide by zero handling
%!test
%! _mvprefl_testhelpers
%! [patches weights a b] = genpatches();
%! weights(1, 1, :) = zeros(1, 1, size(patches)(3));
%! e = mvprefl_fast(patches, weights);
%! assert(isfinite(e));

% Test divide by zero handling
%!test
%! _mvprefl_testhelpers
%! [patches weights a b] = genpatches();
%! weights(:, :, 1) = zeros(size(patches)(1:2));
%! e = mvprefl_fast(patches, weights);
%! assert(isfinite(e));

% vim:set syntax=octave:

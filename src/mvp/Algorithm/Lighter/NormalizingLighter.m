function self = NormalizingLighter()
  self = MvpClass();
  self.light = @light;
endfunction

function obj = light(self, patches, weights)
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
%!
%!
%!
%!
%!

% vim:set syntax=octave:

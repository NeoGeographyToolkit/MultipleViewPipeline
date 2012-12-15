function self = FminbndCorrelator(oic, datum, lighter)
  self=MvpClass();

  self.oic = oic;
  self.datum = datum;
  self.lighter = lighter;

  self.correlate = @correlate;
  self.obj_helper = @obj_helper;
endfunction

function f = obj_helper(self, post, algovar)
  xyz = self.datum.geodetic_to_cartesian([post; algovar.alt()]);
  raw_patches = self.oic.back_project(xyz, algovar.orientation(), algovar.scale(), algovar.window());

  if (numel(raw_patches) > 1)
    patches = zeros(algovar.window()(2), algovar.window()(1), numel(raw_patches));
    weights = ones(algovar.window()(2), algovar.window()(1), numel(raw_patches));
    for i = 1:numel(raw_patches)
      patches(:, :, i) = raw_patches{i};    
    endfor

    idx = find(isnan(patches));
    patches(idx) = 0;
    weights(idx) = 0;

    f = self.lighter.light(patches, weights);
  else
    f = NA;
  endif
endfunction

function r = status_fcn(x, optv, status)
  x
  optv
  status
  r = 0;
endfunction

function result = correlate(self, post, seed)

  try
    opts = optimset("MaxIter", 80, "FunValCheck", "on");
%    opts = optimset("OutputFcn", @status_fcn);
    opts = optimset(opts, "TolX", 1e-2);

    altMin = seed.alt() - 2000;
    altMax = seed.alt() + 2000;

    [alt confidence info output] = fminbnd(@(a) obj_helper(self, post, AlgorithmVar([a; seed.vectorize()(2:end)])), altMin, altMax, opts);
    converged = (info == 1);
    num_iterations = output.iterations;

    result = PixelResult(AlgorithmVar([alt; seed.vectorize()(2:end)]), confidence, converged, num_iterations);
  catch
    result = PixelResult(AlgorithmVar(NA(12, 1)), NA, NA, NA, NA);    
  end_try_catch

endfunction

% vim:set syntax=octave:

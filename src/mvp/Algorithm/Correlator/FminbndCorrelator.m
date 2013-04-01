function self = FminbndCorrelator(_oic, _lighter, _objective)
  self = mvpclass();

  self._oic = _oic;
  self._lighter = _lighter;
  self._objective = _objective;

  self.correlate = @correlate;
endfunction

function r = status_fcn(x, optv, status)
  x
  optv
  status
  r = 0;
endfunction

function f = hack(helper, var)
  f = helper.func(var);
endfunction

function result = correlate(self, post, seed)

  try
    opts = optimset("MaxIter", 80, "FunValCheck", "on");
    opts = optimset("OutputFcn", @status_fcn);
    opts = optimset(opts, "TolX", 1e-2);

    radMin = seed.radius() - 2000;
    radMax = seed.radius() + 2000;

    helper = ObjectiveHelper(self._oic, self._lighter, self._objective, post);

    [radius confidence info output] = fminbnd(@(a) hack(helper, AlgorithmVar([a; seed.vectorize()(2:end)])), radMin, radMax, opts);
    converged = (info == 1);
    num_iterations = output.iterations;

    result = PixelResult(AlgorithmVar([radius; seed.vectorize()(2:end)]), confidence, converged, num_iterations);
  catch
    result = PixelResult(AlgorithmVar(NA(12, 1)), NA, NA, NA);
  end_try_catch

endfunction

% vim:set syntax=octave:

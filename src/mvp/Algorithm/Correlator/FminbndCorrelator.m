function self = FminbndCorrelator(_oic, _lighter, _objective, _settings)
  self = mvpclass();

  self._oic = _oic;
  self._lighter = _lighter;
  self._objective = _objective;
  self._settings = _settings;

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
    opts = optimset("FunValCheck", "on");
    opts = optimset(opts, "MaxIter", self._settings.max_iter);
    opts = optimset(opts, "TolX", self._settings.tol);
    %opts = optimset(opts, "OutputFcn", @status_fcn);

    radMin = seed.radius() - self._settings.alt_search_range;
    radMax = seed.radius() + self._settings.alt_search_range;

    helper = ObjectiveHelper(self._oic, self._lighter, self._objective, post);

    [radius confidence info output] = fminbnd(@(a) hack(helper, AlgorithmVar([a; seed.vectorize()(2:end)])), radMin, radMax, opts);
    converged = (info == 1);
    num_iterations = output.iterations;

    result = PixelResult([radius; seed.vectorize()(2:end); confidence; converged; num_iterations]);
  catch
    result = PixelResult(zeros(globals().PIXELRESULT_LENGTH, 1));
  end_try_catch

endfunction

% vim:set syntax=octave:

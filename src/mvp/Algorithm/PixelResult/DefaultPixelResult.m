function self = PixelResult(_algorithm_var, _confidence, _converged, _num_iterations)
  self = MvpClass();

  if (nargin == 0)
    self._v = zeros(globals().PIXELRESULT_LENGTH, 1);
  elseif (nargin == 1)
    self._v = _algorithm_var(:);
  else
    self._v = [_algorithm_var.vectorize(); _confidence; _converged; _num_iterations];
  endif

  self.algorithm_var = @(self) AlgorithmVar(self._v(1:globals().ALGORITHMVAR_LENGTH));
  self.confidence = @(self) self._v(globals().CONFIDENCE_IDX);
  self.converged = @(self) self._v(globals().CONVERGED_IDX);
  self.num_iterations = @(self) self._v(globals().NUM_ITERATIONS_IDX);
  self.vectorize = @(self) self._v;
endfunction

% vim:set syntax=octave:

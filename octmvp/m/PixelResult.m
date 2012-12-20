function self = PixelResult(_algorithm_var, _confidence, _converged, _num_iterations)
  self = MvpClass();

  self._algorithm_var = _algorithm_var;
  self._confidence = _confidence;
  self._converged = _converged;
  self._num_iterations = _num_iterations;

  self.algorithm_var = @(self) self._algorithm_var;
  self.confidence = @(self) self._confidence;
  self.converged = @(self) self._converged;
  self.num_iterations = @(self) self._num_iterations;
endfunction

% vim:set syntax=octave:

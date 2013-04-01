function self = DefaultPixelResult(v)
  self = mvpclass();

  self._v = v;

  self.algorithm_var = @(self) AlgorithmVar(self._v(1:globals().ALGORITHMVAR_LENGTH));
  self.confidence = @(self) self._v(globals().CONFIDENCE_IDX);
  self.converged = @(self) self._v(globals().CONVERGED_IDX);
  self.num_iterations = @(self) self._v(globals().NUM_ITERATIONS_IDX);
  self.vectorize = @(self) self._v;
endfunction

% vim:set syntax=octave:

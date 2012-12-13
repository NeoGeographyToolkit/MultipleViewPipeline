function self = PixelResult(algorithm_var, confidence, converged, num_iterations)
  self = MvpClass();

  self.malgorithm_var = algorithm_var;
  self.mconfidence = confidence;
  self.mconverged = converged;
  self.mnum_iterations = num_iterations;

  self.algorithm_var = @algorithm_var;
  self.confidence = @confidence;
  self.converged = @converged;
  self.num_iterations = @num_iterations;
endfunction

function a = algorithm_var(self)
  a = self.malgorithm_var;
endfunction

function c = confidence(self)
  c = self.mconfidence;
endfunction

function c = converged(self)
  c = self.mconverged;
endfunction

function n = num_iterations(self)
  n = self.mnum_iterations;
endfunction

% vim:set syntax=octave:

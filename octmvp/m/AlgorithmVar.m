function self = AlgorithmVar(_v)
  if (numel(_v) != 12)
    error("Must construct with vector size 12");
  endif

  self = MvpClass();

  self._v = _v(:);

  self.radius = @(self) self._v(1);
  self.orientation = @(self) self._v(2:5);
  self.window = @(self) self._v(6:7);
  self.gwindow = @(self) self._v(8:9);
  self.smooth = @(self) self._v(10);
  self.gsmooth = @(self) self._v(11);
  self.scale = @(self) self._v(12);
  self.vectorize = @(self) self._v;
endfunction

% vim:set syntax=octave:

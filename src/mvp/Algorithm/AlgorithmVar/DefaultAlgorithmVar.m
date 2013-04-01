function self = AlgorithmVar(_v)
  if (nargin == 0)
    _v = zeros(globals().ALGORITHMVAR_LENGTH, 1);
  endif

  if (numel(_v) != globals().ALGORITHMVAR_LENGTH)
    error("Must construct with vector size 12");
  endif

  self = mvpclass();

  self._v = _v(:);

  self.radius = @(self) self._v(globals().RADIUS_IDX);
  self.orientation = @(self) self._v(globals().ORIENTATION_IDX);
  self.window = @(self) self._v(globals().WINDOW_IDX);
  self.gwindow = @(self) self._v(globals().GWINDOW_IDX);
  self.smooth = @(self) self._v(globals().SMOOTH_IDX);
  self.gsmooth = @(self) self._v(globals().GSMOOTH_IDX);
  self.scale = @(self) self._v(globals().SCALE_IDX);
  self.vectorize = @(self) self._v;
endfunction

% vim:set syntax=octave:

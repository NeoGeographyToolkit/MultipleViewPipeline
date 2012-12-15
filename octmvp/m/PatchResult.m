function self = PatchResult(_patch, _a, _b)
  self = MvpClass();

  self._patch = _patch;
  self._a = _a;
  self._b = _b;
  
  self._corrected_patch = _patch;
  self._corrected_patch(:, :, 1) -= _a;
  if (_b != 0)
    self._corrected_patch(:, :, 1) /= _b;
  endif

  self.patch = @(self) self._patch;;
  self.corrected_patch = @(self) self._corrected_patch;
  self.a = @(self) self._a;
  self.b = @(self) self._b;
endfunction

% vim:set syntax=octave:

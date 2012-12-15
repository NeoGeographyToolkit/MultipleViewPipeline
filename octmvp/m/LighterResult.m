function self = LighterResult(_patches, _albedo, _patch_sum)
  self = MvpClass();

  self._patches = _patches;
  self._albedo = _albedo;
  self._patch_sum = _patch_sum;

  self.patches = @(self) self._patches;
  self.albedo = @(self) self._albedo;
  self.patch_sum = @(self) self._patch_sum;
endfunction

% vim:set syntax=octave:

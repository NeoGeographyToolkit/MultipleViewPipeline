function self = DefaultAlbedoBox(_patch_box, _a, _b)
  self = mvpclass();

  dim = [_patch_box.rows() _patch_box.cols()];
  num_patches = _patch_box.planes();

  arep = repmat(reshape(_a, [1 1 num_patches]), [dim 1]);
  brep = repmat(reshape(_b, [1 1 num_patches]), [dim 1]);

  self._albedo = _patch_box.intensity() .* arep + brep;
  self._weight = _patch_box.weight();

  % self._xgrad_albedo = _patch_box.xgrad_intensity() .* arep;
  % self._ygrad_albedo = _patch_box.ygrad_intensity() .* arep;
  % self._xgrad_weight = _patch_box.xgrad_weight();
  % self._ygrad_weight = _patch_box.ygrad_weight();

  self._weight_sum = _patch_box.weight_sum();
  self._weight_sum2 = _patch_box.weight_sum2();

  self._global_albedo = sum(self._albedo, 3) ./ self._weight_sum;

  self._a = _a;
  self._b = _b;

  self.albedo = @(self) self._albedo;
  self.weight = @(self) self._weight;

  self.global_albedo = @(self) self._global_albedo;
  self.a = @(self) self._a;
  self.b = @(self) self._b;

  self.weight_sum = @(self) self._weight_sum;
  self.weight_sum2 = @(self) self._weight_sum2;
  
  self.rows = @(self) dim(1);
  self.cols = @(self) dim(2);
  self.planes = @(self) num_patches;
endfunction

% vim:set syntax=octave:

function self = DefaultPatchBox(_patches, _gwindow, _smooth, _gsmooth)
  self = mvpclass();

  dim = size(_patches{1});
  num_patches = numel(_patches);

  self._intensity = zeros(dim(1), dim(2), num_patches);
  for i = 1:num_patches
    self._intensity(:, :, i) = _patches{i};
  endfor

  idx = isna(self._intensity);
  self._weight = !idx;
  self._intensity(idx) = 0;

  self._intensity2 = self._intensity .* self._intensity;

  % TODO: Smooth weight+intensity+intensity2 with gsmooth+smooth

  % TODO: Create gaussian window
  self._window = ones(dim);

  % Calculate weight sums
  self._weight_sum = sum(self._weight, 3);
  self._weight_sum2 = sum(self._weight .* self._weight, 3);

  self.window = @(self) self._window;
  self.intensity = @(self) self._intensity;
  self.intensity2 = @(self) self._intensity2;
  self.weight = @(self) self._weight;
  self.weight_sum = @(self) self._weight_sum;
  self.weight_sum2 = @(self) self._weight_sum2;

  self.rows = @(self) dim(1);
  self.cols = @(self) dim(2);
  self.planes = @(self) num_patches;
endfunction

% vim:set syntax=octave:

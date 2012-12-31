function self = TileResult(_georef, _size)
  self = MvpClass();

  self._georef = _georef;
  self._size = _size;

  self._data = zeros(_size(2), _size(1), 15);

  self.set = @set;
  self.get = @get;
  self.plate_layer = @plate_layer;
endfunction

function set(self, pixel, value)
  self._data(pixel(2)+1, pixel(1)+1,:) = [value.algorithm_var().vectorize(); \
                                          value.confidence(); value.converged(); \
                                          value.num_iterations()];
endfunction

function value = get(self, pixel)
  raw = self._data(pixel(2)+1, pixel(1)+1, :);
  value = PixelResult(AlgorithmVar(raw(1:12)), raw(13), raw(14), raw(15));
endfunction

function layer = plate_layer(self, num)
  layer = self._data(:,:,num+1);
endfunction

% vim:set syntax=octave:

function self = TileResult(_georef, _size)
  self = MvpClass();

  self._georef = _georef;
  self._size = _size;

  self._data = zeros(_size(2), _size(1), 15);

  self.set = @set;
  self.get = @get;
  self.plate_layer = @(self, num) self._data(:,:,num+1);

  self.alt = @(self) self._data(:,:,1) - self._georef.datum().semi_major_axis(); 
  self.orientation = @(self) self._data(:,:,2:5); 
  self.window = @(self) self._data(:,:,6:7);
  self.gwindow = @(self) self._data(:,:,8:9);
  self.smooth = @(self) self._data(:,:,10);
  self.gsmooth = @(self) self._data(:,:,11);
  self.scale = @(self) self._data(:,:,12);
  self.confidence = @(self) self._data(:,:,13);
  self.converged = @(self) self._data(:,:,14);
  self.num_iterations = @(self) self._data(:,:,15);
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

% vim:set syntax=octave:

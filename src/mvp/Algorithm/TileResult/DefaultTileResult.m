function self = TileResult(_georef, _size)
  self = mvpclass();

  self._georef = _georef;
  self._size = _size;

  self._data = zeros(_size(2), _size(1), globals().PIXELRESULT_LENGTH);

  self.set = @(self, pixel, value) self._data(pixel(2)+1, pixel(1)+1,:) = value.vectorize();
  self.get = @(self, pixel) PixelResult(self._data(pixel(2)+1,pixel(1)+1,:));
  self.plate_layer = @(self, num) self._data(:,:,num+1);

  self.alt = @(self) self._data(:,:,globals().RADIUS_IDX) - self._georef.datum().semi_major_axis(); 
  self.orientation = @(self) self._data(:,:,globals().ORIENTATION_IDX); 
  self.window = @(self) self._data(:,:,globals().WINDOW_IDX);
  self.gwindow = @(self) self._data(:,:,globals().GWINDOW_IDX);
  self.smooth = @(self) self._data(:,:,globals().SMOOTH_IDX);
  self.gsmooth = @(self) self._data(:,:,globals().GSMOOTH_IDX);
  self.scale = @(self) self._data(:,:,globals().SCALE_IDX);
  self.confidence = @(self) self._data(:,:,globals().CONFIDENCE_IDX);
  self.converged = @(self) self._data(:,:,globals().CONVERGED_IDX);
  self.num_iterations = @(self) self._data(:,:,globals().NUM_ITERATIONS_IDX);
endfunction

% vim:set syntax=octave:

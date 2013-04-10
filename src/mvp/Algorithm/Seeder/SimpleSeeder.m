function self = SimpleSeeder(_georef, _size, _settings)
  self = mvpclass();

  self._georef = _georef;
  % IMPORTANT:
  % Size comes in as a Vector2i, turn into double
  % so operations work as expected (like center pixel)
  self._size = double(_size);
  self._settings = _settings;

  center_pixel = (self._size - 1) / 2;

  self._curr_lonlat = self._georef.pixel_to_lonlat(center_pixel);
  self._curr_post = pixel2post(self._georef, center_pixel);
  self._curr_seed = AlgorithmVar([self._settings.alt+_georef.datum().semi_major_axis();
                                  tanplane(self._curr_post); 
                                  [self._settings.window_x;self._settings.window_y]; 
                                  [self._settings.gwindow_x;self._settings.gwindow_y];
                                  self._settings.smooth;
                                  self._settings.gsmooth;
                                  self._settings.scale]);
  self._done = 0;
  self._result = {};

  self.curr_post = @(self) self._curr_post;
  self.curr_seed = @(self) self._curr_seed;
  self.done = @(self) self._done; 
  self.result = @(self) self._result;
  self.update = @update;
endfunction

function update(self, new_result)
  v = new_result.vectorize();

  v(globals().WINDOW_IDX(1):globals().SCALE_IDX) = [self._settings.out_window_x;
                                                    self._settings.out_window_y;
                                                    self._settings.out_gwindow_x;
                                                    self._settings.out_gwindow_y;
                                                    self._settings.out_smooth;
                                                    self._settings.out_gsmooth;
                                                    self._settings.out_scale];

  self._result = {Seed(self._curr_lonlat, PixelResult(v))};
  self._done = 1;
endfunction

% vim:set syntax=octave:

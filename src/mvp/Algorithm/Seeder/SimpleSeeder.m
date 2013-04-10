function self = SimpleSeeder(_georef, _size)
  self = mvpclass();

  self._georef = _georef;
  % IMPORTANT:
  % Size comes in as a Vector2i, turn into double
  % so operations work as expected (like center pixel)
  self._size = double(_size);

  center_pixel = (self._size - 1) / 2;

  self._curr_lonlat = self._georef.pixel_to_lonlat(center_pixel);
  self._curr_post = pixel2post(self._georef, center_pixel);
  self._curr_seed = AlgorithmVar([-1000+_georef.datum().semi_major_axis();
                                  tanplane(self._curr_post); [30;30]; [0;0];
                                  0; 0; 60]);
  self._done = 0;
  self._result = {};

  self.curr_post = @(self) self._curr_post;
  self.curr_seed = @(self) self._curr_seed;
  self.done = @(self) self._done; 
  self.result = @(self) self._result;
  self.update = @update;
endfunction

function update(self, new_result)
  self._result = {Seed(self._curr_lonlat, new_result)};
  self._done = 1;
endfunction

% vim:set syntax=octave:

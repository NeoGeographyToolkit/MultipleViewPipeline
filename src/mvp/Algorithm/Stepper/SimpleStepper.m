function self = SimpleStepper(_georef, _size, _seed_list)
  self = MvpClass();

  if (_size(1) != _size(2))
    error("SimpleStepper only works on square tiles");
  endif

  self._result = {};
  self._georef = _georef;
  self._size = _size;
  self._seed_list = _seed_list;

  self._curr_direction = 1;

  self._curr_pixel = self._size / 2;
  %self._curr_seed = self._seed_list{1}.seed();
  self._done = 0;
  
  self.curr_pixel = @(self) self._curr_pixel;
  self.curr_post = @curr_post;
  self.curr_seed = @(self) self._curr_seed;
  self.done = @(self) self._done;
  self.update = @update;
  self.result = @(self) self._result;
endfunction

function post = curr_post(self)
  post = pixel2post(self._georef, self._curr_pixel);
endfunction

function update(self, new_result)
  % init directions
  directions = {[-1;0],[0;-1],[1;0],[0;1]};
  num_directions = numel(directions);

  % store result
  self._result{self._curr_pixel(2) + 1, self._curr_pixel(1) + 1} = new_result;

  % step curr_pixel
  self._curr_pixel += directions{self._curr_direction};

  % check if direction should be shanged
  if (sum(self._curr_pixel) == self._size(1) - 1 || self._curr_pixel(1) == self._curr_pixel(2))
    self._curr_direction += 1;
  endif

  % check if direction needs to be reset
  if (self._curr_direction > num_directions)
    self._curr_pixel += [1;1];
    self._curr_direction = 1;
  endif

  % check if done
  if (any(self._curr_pixel == self._size))
    self._done = 1;
  endif

  % if not done, update curr_seed
  if (!self._done)
    % TODO            
  endif
endfunction

% vim:set syntax=octave:

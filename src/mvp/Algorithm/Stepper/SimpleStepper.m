function self = SimpleStepper(_georef, _size, _seed_list)
  self = MvpClass();

  if (_size(1) != _size(2))
    error("SimpleStepper only works on square tiles");
  endif

  self._result = TileResult(_georef, _size);
  self._georef = _georef;
  self._size = _size;
  self._seed_list = _seed_list;

  self._curr_direction = 1;

  self._curr_pixel = self._size / 2;
  self._curr_seed = self._seed_list{1}.value().algorithm_var();
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
  self._result.set(self._curr_pixel, new_result);

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
  if (any(self._curr_pixel >= self._size))
    self._done = 1;
  endif

  % if not done, update curr_seed
  if (!self._done)
    varsum = AlgorithmVar().vectorize();
    num = 0;
    for row = (-1:1) + self._curr_pixel(2)
      for col = (-1:1) + self._curr_pixel(1)
        if (all([col;row] < self._size) && all([col;row] >= [0;0]))
          if (self._result.get([col;row]).converged())
            varsum += self._result.get([col;row]).algorithm_var().vectorize();
            num += 1;
          endif 
        endif
      endfor
    endfor
    if (num)
      self._curr_seed = AlgorithmVar(varsum / num);
    else
      self._curr_seed = self._seed_list{1}.value().algorithm_var();
    endif
  endif
endfunction

% TODO: Test edge cases

%!
%!test
%!  stepper = SimpleStepper([], [64;64], {Seed([], PixelResult())});
%!  assert(stepper.curr_pixel(), [32; 32]);
%!  assert(stepper.curr_seed().vectorize(), zeros(12,1));
%!  
%!  stepper.update(PixelResult(AlgorithmVar(zeros(12,1)), 1, 1, 1));
%!  assert(stepper.curr_pixel(), [31; 32]);
%!  assert(stepper.curr_seed().vectorize(), zeros(12,1));
%!
%!  stepper.update(PixelResult(AlgorithmVar(ones(12,1)), 1, 1, 1));
%!  assert(stepper.curr_pixel(), [31; 31]);
%!  assert(stepper.curr_seed().vectorize(), 0.5 * ones(12,1));

% vim:set syntax=octave:

function self = OrbitalImageCollection(im)
  self = MvpClass();

  self.im = im;
  self.project = @back_project;
endfunction

function result = back_project(self, xyz, orientation, scale, sz)
  cursor = 1;
  for j = 1:numel(self.im);
    patch = _back_project(self.im(j).data, self.im(j).camera, xyz, orientation, [scale scale], sz);
    if any(patch(:))
      result{cursor} = patch;
      cursor += 1;
    endif
  endfor
endfunction

% vim:set syntax=octave:

function self = NormalizingLighter()
  self = MvpClass();
  self.light = @light;
endfunction

function albedo_box = light(self, patch_box)
  dim = [patch_box.rows() patch_box.cols()];
  num_patches = patch_box.depth();

  si = sum(sum(patch_box.intensity()))(:);
  si2 = sum(sum(patch_box.intensity2()))(:);
  sw = sum(sum(patch_box.weight()))(:);

  a = si ./ sw; % mean
  b = (si2 .* sw - si .* si) ./ sw ./ sw; % stddev

  albedo_box = AlbedoBox(patch_box, a, b);
endfunction

%!test
%!
%!
%!
%!
%!

% vim:set syntax=octave:

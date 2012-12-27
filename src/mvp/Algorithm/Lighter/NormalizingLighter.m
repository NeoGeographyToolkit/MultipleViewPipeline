function self = NormalizingLighter()
  self = MvpClass();
  self.light = @light;
endfunction

function albedo_box = light(self, patch_box)
  dim = [patch_box.rows() patch_box.cols()];
  num_patches = patch_box.planes();

  si = sum(sum(patch_box.intensity()))(:);
  si2 = sum(sum(patch_box.intensity2()))(:);
  sw = sum(sum(patch_box.weight()))(:);

  patch_mean = si ./ sw; 
  patch_stddev = (si2 .* sw - si .* si) ./ sw ./ sw;

  prevWarnState = warning("query", "Octave:divide-by-zero");
  warning("off", "Octave:divide-by-zero");

  a = 1 ./ patch_stddev;
  a(isnan(a)) = 0;

  warning(prevWarnState.state, prevWarnState.identifier);

  b = -patch_mean .* a;

  albedo_box = AlbedoBox(patch_box, a, b);
endfunction

%!test
%!
%!
%!
%!
%!

% vim:set syntax=octave:

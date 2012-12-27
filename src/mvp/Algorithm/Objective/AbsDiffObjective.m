function self = AbsDiffObjective()
  self = MvpClass();
 
  self.func = @func;
  self.grad = @grad; 
endfunction

function f = func(self, albedo_box)
    dim = [albedo_box.rows() albedo_box.cols()];
    num_patches = albedo_box.planes();

    absdiffs = abs(albedo_box.albedo() - repmat(albedo_box.global_albedo(), [1 1 num_patches]));

    f = sum(absdiffs(:)) / sum(albedo_box.weight_sum()(:));
endfunction

function grad(self)
  error("AbsDiffObjective does not support a gradient");
endfunction

% vim:set syntax=octave:

function self = AbsDiffObjective()
  self = MvpClass();
 
  self.func = @func;
  self.grad = @grad; 
endfunction

function f = func(self, lighter_result)
    dim = size(lighter_result.patches(){1}.corrected_patch());
    num_patches = numel(lighter_result.patches());

    diffs = zeros(dim(1), dim(2));
    for i = 1:num_patches
      diffs += abs(lighter_result.patches(){i}.corrected_patch()(:, :, 1) - lighter_result.albedo()) \
               .* lighter_result.patches(){i}.corrected_patch()(:, :, 2);
    endfor

    f = sum(diffs(:)) ./ sum(lighter_result.patch_sum()(:, :, 2)(:));
endfunction

function grad(self)
  error("AbsDiffObjective does not support a gradient");
endfunction

% vim:set syntax=octave:

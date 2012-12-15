function self = NormalizingLighter()
  self = MvpClass();
  self.light = @light;
endfunction

function lighter_result = light(self, weighted_patches)
  dim = size(weighted_patches{1});
  num_patches = numel(weighted_patches);

  for i = 1:num_patches
    swx = sum(weighted_patches{i}(:, :, 1)(:) .* weighted_patches{i}(:, :, 2)(:));
    swx2 = sum(weighted_patches{i}(:, :, 1)(:) .* weighted_patches{i}(:, :, 1)(:) .* weighted_patches{i}(:, :, 2)(:));
    sw = sum(weighted_patches{i}(:, :, 2)(:));

    a = swx / sw; % mean
    b = (swx2*sw-swx*swx) / sw / sw; % stddev
    result_patches{i} = PatchResult(weighted_patches{i}, a, b);
  endfor

  patch_sum = zeros(dim(1), dim(2), 2);
  for i = 1:num_patches
    patch_sum += result_patches{i}.patch();
  endfor

  albedo = patch_sum(:, :, 1) ./ patch_sum(:, :, 2);

  lighter_result = LighterResult(result_patches, albedo, patch_sum);

endfunction

%!test
%!
%!
%!
%!
%!

% vim:set syntax=octave:

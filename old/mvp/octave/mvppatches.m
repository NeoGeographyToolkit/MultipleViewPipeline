function [patches weights] = mvppatches(projs, windows, mvpoptions)
  % Given projections, smooth them, create weight images

  dim = size(projs{1});
  numProjs = numel(projs);
  windows_px = round(windows * mvpoptions.gauss_divisor);

  smoothKernel = gausskernel(windows(3), windows_px(3));

  numPatches = 0;
  patches = zeros(0, 0, 0);
  for i = 1:numProjs
    if (all(isnan(projs{i})))
      continue;
    endif

    projs{i} = convn(projs{i}, smoothKernel, "valid");
    projs{i} = convn(projs{i}, smoothKernel', "valid");

    if (all(isnan(projs{i})))
      continue;
    endif

    numPatches += 1;
    patches(:, :, numPatches) = projs{i};
  endfor

  weightWin = gausskernel(windows(1), windows_px(1))' * gausskernel(windows(2), windows_px(2));
  weights = repmat(weightWin, [1 1 numPatches]);

  idx = find(isnan(patches));
  patches(idx) = 0;
  weights(idx) = 0;

endfunction

%% TODO: test me!

% vim:set syntax=octave:

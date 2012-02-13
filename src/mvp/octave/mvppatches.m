function [patches weights] = mvppatches(projs, windows)
  % Given projections, smooth them, create weight images

  if (ischar(projs) && projs == "projsize")
    patchSize = gausskernel("kernsize", windows(1:2));
    paddingSize = gausskernel("kernsize", windows(3));
    patches = patchSize + (paddingSize - 1);
    return;
  endif

  dim = size(projs{1});
  numProjs = numel(projs);

  smoothKernel = gausskernel(windows(3));

  numPatches = 0;
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

  weightWin = gausskernel(windows(1))' * gausskernel(windows(2));
  weights = repmat(weightWin, [1 1 numPatches]);

  idx = find(isnan(patches));
  patches(idx) = 0;
  weights(idx) = 0;

endfunction

%% TODO: test me!

% vim:set syntax=octave:

function obj = mvpobj(alt, orientation, windows, georef, images, mvpoptions)
  patches = mvppatches(images, georef, alt, orientation, windows);

  numPatches = numel(patches);

  % Normalize patches and find albedo
  numValidPatches = 0;
  albedo = zeros(size(patches{1}));
  for k = 1:numPatches
    valid{k} = find(!isnan(patches{k}));
    if (sum(valid{k}) == 0)
      continue;
    endif
    stddev = std(patches{k}(valid{k}));
    if (stddev == 0)
      patches{k} = zeros(size(patches{k}));
    else
      patches{k} = (patches{k} - mean(patches{k}(valid{k}))) / stddev;
    endif
    albedo(valid{k}) += patches{k}(valid{k});
    numValidPatches++;
  endfor
  albedo /= numPatches;

  if (numValidPatches < 2)
    obj = NA;
    return;
  endif

  % Add errors
  obj = 0;
  for k = 1:numPatches
    if (sum(valid{k}) == 0)
      continue;
    endif
    obj += sum(abs(albedo - patches{k})(valid{k})) / sum(valid{k});
  endfor

endfunction

% vim:set syntax=octave:

1; % Prevent octave from thinking this is a function .m

function e = objscat(patches, weights, a, b)
  sz = size(patches);
  dim = sz(1:2);
  numPatches = sz(3);

  [Haa Hab Hbb] = _mvprefl_findH(patches, weights);

  H = [Haa Hab; Hab' Hbb];

  e = [a;b]'*H*[a;b];  
endfunction

function e = objref(patches, weights, a, b)
  sz = size(patches);
  dim = sz(1:2);
  numPatches = sz(3);

  patches .*= repmat(reshape(a, [1 1 numPatches]), [dim 1]);
  patches .+= repmat(reshape(b, [1 1 numPatches]), [dim 1]);

  sums = sum(weights .* patches, 3);
  sumSquares = sum(weights .* patches .* patches, 3);
  weightSum = sum(weights, 3);

  e = sum((weightSum .* sumSquares - sums .^ 2)(:)) / numPatches;
endfunction

function [patches weights a b] = genpatches(dim = [11 11], numPatches = 4, seed = 10)
  rand("seed", seed);

  a = rand(4, 1);
  b = rand(4, 1);

  albedo = rand(dim);

  patches = repmat(albedo, [1 1 numPatches]);
  patches .-= repmat(reshape(b, [1 1 numPatches]), [dim 1]);
  patches ./= repmat(reshape(a, [1 1 numPatches]), [dim 1]);

  weights = rand([dim numPatches]);
endfunction

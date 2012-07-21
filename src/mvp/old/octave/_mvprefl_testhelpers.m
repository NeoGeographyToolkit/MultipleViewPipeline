1; % Prevent octave from thinking this is a function .m

function obj = mvprefl_fast_ref(patches, weights)
  sz = size(patches);
  dim = sz(1:2);
  numPatches = sz(3);

  albedo = zeros(dim);
  for i = 1:numPatches
    currPatch = patches(:, :, i);
    currWeight = weights(:, :, i);
    weightSum = sum(currWeight(:));

    meanVal = sum(currPatch(:) .* currWeight(:)) / weightSum;

    sqDevPatch = (currPatch - meanVal) .^ 2;

    stddev = sqrt(sum(sqDevPatch(:) .* currWeight(:)) / weightSum);

    patches(:, :, i)  = (currPatch - meanVal) ./ stddev;
    albedo += patches(:, :, i) .* currWeight;
  endfor
  
  albedo ./= sum(weights, 3);

  diffs = zeros(dim);
  for i = 1:numPatches
    diffs += abs(albedo - patches(:, :, i)) .* weights(:, :, i);
  endfor

  obj = sum(diffs(:)) / sum(weights(:));
endfunction

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

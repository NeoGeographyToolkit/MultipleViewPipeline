function [Haa Hab Hbb] = _mvprefl_findH(patches, weights)
  % TODO: replace these calculations with Taemin's fancy
  % indexed versions

  sz = size(patches);
  Haa = scat(patches, patches, weights);
  Hab = scat(patches, ones(sz), weights);
  Hbb = scat(ones(sz), ones(sz), weights);
endfunction

function H = scat(f, g, w)
  sz = size(f);
  dim = sz(1:2);
  numPatches = sz(3);

  H1 = H2 = zeros(numPatches);
  for row = 1:sz(1)
    for col = 1:sz(2)
      F = diag(f(row, col, :)(:));
      G = diag(g(row, col, :)(:));
      W = diag(w(row, col, :)(:));
      H1 += sum(w(row, col, :)) * F * G * W;
      H2 += W * f(row, col, :)(:) * g(row, col, :)(:)' * W;
    endfor
  endfor
  H1 /= numPatches;
  H2 /= numPatches;

  H = H1 - H2;
endfunction

%
% The following test tests the calculations of Haa, Hab, and Hbb
% by using them to calculate the objective function (objscat), and then
% comparing it to a reference calculation (objref)
%

%!test
%! _mvprefl_testhelpers
%! [patches weights a b] = genpatches();
%! assert(objref(patches, weights, a, b), 0, 1e-8);
%! assert(objscat(patches, weights, a, b), objref(patches, weights, a, b), 1e-8);
%! a = rand(size(a));
%! b = rand(size(b));
%! assert(objscat(patches, weights, a, b), objref(patches, weights, a, b), 1e-8);

% vim:set syntax=octave:

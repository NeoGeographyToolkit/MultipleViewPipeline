function [kern dkern] = gausskernel(sigma, sz)
  if (sigma == "kernsize")
    % A gaussian kernel needs 6sigma values
    kern = round(6 * sz);
    kern(find(kern<=0)) = 1;
    return;
  endif

  if (sigma <= 0)
    kern = 1;
    return;
  endif

  if (nargin < 2)
    sz = gausskernel("kernsize", sigma);
  endif

  x = (0:sz) - (sz / 2);
  kern = diff(normcdf(x, 0, sigma));
  kern /= sum(kern);

  if (nargout > 1)
    dkern = diff(normpdf(x, 0, sigma));
    dkern /= sum(kern);
  endif 

endfunction

%!test
%! sz = 8;
%! sigma = 3;
%! h = sz - 1;
%! x = (0:h) - h / 2;
%! gauss = exp(-(x.^2) / (2*sigma^2));
%! kernref = gauss / sum(gauss(:));
%! kern = gausskernel(sigma, sz);
%! assert(kernref, kern, 1e-3)

% vim:set syntax=octave:

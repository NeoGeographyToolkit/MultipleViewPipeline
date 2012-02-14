function kern = gausskernel(sigma, sz)
  if (sigma == "kernsize")
    % A gaussian kernel needs 6sigma - 1 values
    kern = ceil(6 * sz - 1);
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

  h = sz - 1;
  x = (0:h) - h / 2;
  gauss = exp(-(x.^2) / (2*sigma^2));
  kern = gauss / sum(gauss(:)); 
endfunction

%%TODO: test me?

% vim:set syntax=octave:

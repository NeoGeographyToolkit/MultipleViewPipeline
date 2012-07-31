function patch = porthoproj(image, planeNormal, planeD, georef, projSize)
  [xw yw] = _porthoproj_impl(image.camera, planeNormal, planeD, georef, projSize);
  patch = imremap(image.data, xw, yw, "bilinear");
endfunction

% vim:set syntax=octave:

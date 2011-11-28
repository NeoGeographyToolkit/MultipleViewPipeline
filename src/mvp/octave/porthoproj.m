function patch = porthoproj(image, planeNormal, planeD, georef, hWin)
  [xw yw] = _porthoproj_impl(image.camera, planeNormal, planeD, georef, hWin);
  patch = imremap(image.data, xw, yw, "bilinear");
endfunction

% vim:set syntax=octave:

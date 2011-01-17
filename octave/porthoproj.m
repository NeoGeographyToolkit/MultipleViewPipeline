function patch = porthoproj(planeNormal, planeD, demPt, georef, orbit, hWin)
  [xw yw] = _porthoproj_impl(planeNormal, planeD, demPt, georef, orbit, hWin);
  patch = imremap(orbit.img, xw, yw, "bilinear");
endfunction

function patch = porthoproj(planeNormal, planeD, demPt, georef, orbit, hWin)
  [xw yw] = _porthoproj_impl_ref(planeNormal, planeD, demPt, georef, orbit, hWin);
  patch = imremap(orbit.img, xw, hw, "bilinear");
endfunction

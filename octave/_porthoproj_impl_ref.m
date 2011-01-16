function [xw yw] = _porthoproj_impl_ref(planeNormal, planeD, demPt, georef, orbit, hWin)

  dim = 2 * hWin + 1;
  
  [patchLon patchLat] = lonlatgrid(georef, dim, demPt - hWin);
 
  patchRad = planedem(planeNormal, planeD, patchLon, patchLat);

  [xx yy zz] = lonlatrad2xyz(patchLons, patchLats, patchRads);

  [xw yw] = _orthoproj_impl(xx, yy, zz, orbit);

endfunction

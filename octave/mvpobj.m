function result = mvpobj(plane, demPt, georef, orbits, hWin, hKern, errfun = "gaussian")
  patches = mvpproj(plane, demPt, georef, orbits, hWin);
  result = mvperr(patches, hKern, errfun);
endfunction

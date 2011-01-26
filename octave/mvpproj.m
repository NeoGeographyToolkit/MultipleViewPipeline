function patches = mvpproj(plane, demPt, georef, orbits, hWin)
  [planeNormal planeD] = plane2normd(plane, demPt, georef);

  patches = cellfun(@(orbit) porthoproj(planeNormal, planeD, demPt, georef, orbit, hWin), 
                    orbits, "UniformOutput", false);
endfunction

function patches = mvpproj(plane, demPt, georef, orbits, hWin)
  cntrPtLonLatH = georef * [demPt(1); demPt(2); 1];
  cntrPtXYZ = plane(3) * lonlat2normal(cntrPtLonLatH(1:2) / cntrPtLonLatH(3));
  planeNormal = lonlat2normal(plane(1:2));
  planeD = dot(planeNormal, cntrPtXYZ);


  patches = cellfun(@(orbit) porthoproj(planeNormal, planeD, demPt, georef, orbit, hWin), 
                    orbits, "UniformOutput", false);
endfunction

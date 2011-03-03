function [planeNormal planeD] = plane2normd(plane, demPt, georef)
  cntrPtLonLatH = georef * [demPt(1); demPt(2); 1];
  cntrPtXYZ = plane(3) * lonlat2normal(cntrPtLonLatH(1:2) / cntrPtLonLatH(3));
  planeNormal = lonlat2normal(plane(1:2));
  planeD = dot(planeNormal, cntrPtXYZ);
endfunction

% vim:set syntax=octave:

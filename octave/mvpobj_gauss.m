% Plane is in the form [dirLon dirLat rad]
function o = mvpobj_gauss(plane, patchLon, patchLat, orbits)
  n = numel(orbits);
  dim = size(orbits{1});

  % Calculate the XYZ location of the center of the patch
  % given the lat lon of the center of the patch and the
  % current estimated radius
  cntrPt = ceil(dim / 2);
  cntrPtLonLat = [patchLon(cntrPt(1), cntrPt(2)); 
                  patchLat(cntrPt(1), cntrPt(2))];
  cntrPtXYZ = plane(3) * lonlat2normal(cntrPtLonLat);

  % Next calculate nx ny nz and d for the plane
  planeNormal = lonlat2normal(plane(1:2));
  planeD = dot(planeNormal, cntrPtXYZ);

  % Generate the plane dem 
  patchRad = planedem(planeNormal, planeD, patchLon, patchLat); 

  % Convert into xyz and orthoproject  
  [xx yy zz] = lonlatrad2xyz(patchLon, patchLat, patchRad);
  patches = cellfun(@(orbit) orthoproj(xx, yy, zz, orbit), orbits, "UniformOutput", false);

  % Find the albedo
  meanpatch = zeros(dim);
  for k = 1:n
    meanpatch += patches{k};
  endfor
  meanpatch /= n;

  % Find the sum of square error
  err = 0;
  for k = 1:n
    err += sum(((meanpatch - patches{k}).^2)(:));
  endfor

  o = err;
endfunction

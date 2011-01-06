% Plane for initial-DEM.tif: Vector4(0.545701763592,0.821530211924,0.16522014438,1734795)
% Plane for ground-DEM.tif: Vector4(0.545430300437,0.821121535979,0.168122605606,1734787.48401)


pt = [20 20]';
plane = [0.545430300437 0.821121535979 0.168122605606 1734787.48401];
halfKern = 7;

plon = ccrop(demLon, pt, halfKern);
plat = ccrop(demLat, pt, halfKern);

prad = ccrop(demGround, pt, halfKern);
prad2 = planedem(plane, plon, plat);

[xx yy zz] = lonlatrad2xyz(plon, plat, prad);

patches = cellfun(@(orbit) orthoproj(xx, yy, zz, orbit), orbits, "UniformOutput", false);

% Plane for initial-DEM.tif: Vector4(0.545702,0.82153,0.16522,1.7348e+06)
% Plane for ground-DEM.tif: Vector4(0.54543,0.821122,0.168123,1.73479e+06)

pt = [20; 20];
halfKern = 7;

plon = ccrop(demLon, pt, halfKern);
plat = ccrop(demLat, pt, halfKern);
prad = ccrop(demGround, pt, halfKern);

[xx yy zz] = lonlatrad2xyz(plon, plat, prad);

patches = cellfun(@(orbit) orthoproj(xx, yy, zz, orbit), orbits, "UniformOutput", false);

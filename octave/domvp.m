% Plane for initial-DEM.tif: Vector4(0.545701763592,0.821530211924,0.16522014438,1734795)
% Plane for ground-DEM.tif: Vector4(0.545430300437,0.821121535979,0.168122605606,1734787.48401)

% Radius at 20, 20 on dem: 1734793.07299805
% planeLat, planeLon = [0.98447 0.16892]

pt = [20; 20];
halfKern = 7;

plane = [0.98447 0.16892 1734793.07299805];

patchLon = ccrop(demLon, pt, halfKern);
patchLat = ccrop(demLat, pt, halfKern);

mvpobj_gauss(plane, patchLon, patchLat, orbits)

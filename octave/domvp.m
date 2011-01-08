% Plane for initial-DEM.tif: Vector4(0.545701763592,0.821530211924,0.16522014438,1734795)
% Plane for ground-DEM.tif: Vector4(0.545430300437,0.821121535979,0.168122605606,1734787.48401)

% Radius at 20, 20 on dem: 1734793.07299805
% planeLat, planeLon = [0.98447 0.16892]

pt = [20; 20];
halfKern = 7;

% plane = [0.98447 0.16892 1734793.07299805];

patchLon = ccrop(demLon, pt, halfKern);
patchLat = ccrop(demLat, pt, halfKern);

%initialPlane = 1734800;
%[x obj info iter nf lambda] = sqp(initialPlane, @(x) mvpobj_gauss([0.98447 0.16892 x], patchLon, patchLat, orbits), [], [], [], [], 10, 1e-20)

%initialPlane = [0.98; 0.17];
%[x obj info iter nf lambda] = sqp(initialPlane, @(x) mvpobj_gauss([x; 1734793.07299805], patchLon, patchLat, orbits), [], [], [], [], 10, 1e-20)

%initialPlane = [0.98; 0.17; 1734800];
%[x obj info iter nf lambda] = sqp(initialPlane, @(x) mvpobj_gauss(x, patchLon, patchLat, orbits), [], [], [], [], 10, 1e-20)

%initialPlane = [0.98; 0.17];
%function o = obj(plane, patchLon, patchLat, orbits)
%  o = mvpobj_gauss([plane; 1734792.07299805], patchLon, patchLat, orbits);
%endfunction
%[x v nev] = minimize("obj", {initialPlane, patchLon, patchLat, orbits}, "maxev", 54, "verbose", "isz", 0.004)

%initialPlane = [0.9; 0.1];
%opts(1) = 1; % show progress
%opts(10) = 40; % Max num of f evals
%fmins(@(x) mvpobj_gauss([x; 1734793.07299805], patchLon, patchLat, orbits), initialPlane, opts)

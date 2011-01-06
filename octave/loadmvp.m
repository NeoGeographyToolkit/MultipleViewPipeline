synthSceneDir = "/home/khusmann/projects/MultipleViewPipeline/synth_scene/";

DATUM_RADIUS = 1737400; % Moon

% Load DEMs
[demGround georef] = imread_vw([synthSceneDir "ground-DEM.tif"]);
demInitial = imread_vw([synthSceneDir "initial-DEM.tif"]);
drgGround = imread_vw([synthSceneDir "ground-DRG.tif"]);

% Add radius into DEMs
demGround += DATUM_RADIUS;
demInitial += DATUM_RADIUS;

% Load orbits
for n = 0:3
  orbits{n+1}.img = imread_vw([synthSceneDir num2str(n) ".tif"]);
  orbits{n+1}.cam = loadcam_vw([synthSceneDir num2str(n) ".pinhole"]);
end

% Build lonlat grid
[demLon demLat] = lonlatgrid(georef, size(demGround));

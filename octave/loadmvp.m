synthSceneDir = "/home/khusmann/projects/MultipleViewPipeline/synth_scene/";

DATUM_RADIUS = 1737400; % Moon

[demGround georef] = imread_vw([synthSceneDir "ground-DEM.tif"]);
% Add radius into DEM
demGround += DATUM_RADIUS;
% Degrees to radians
georef = diag([pi/180 pi/180 1]) * georef;

[demLon demLat] = lonlatgrid(georef, size(groundDEM));

for n = 0:3
  orbits{n+1}.img = imread_vw([synthSceneDir num2str(n) ".tif"]);
  orbits{n+1}.cam = loadcam_vw([synthSceneDir num2str(n) ".pinhole"]);
end

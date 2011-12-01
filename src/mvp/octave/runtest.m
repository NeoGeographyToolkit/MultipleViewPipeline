%% Find a good plane estimate
#patchPt = [80 30];
#patchPt = [215 165];
patchPt = [250 250];

patch_georef = offsetgeoref(tile_georef, patchPt);

lonlatPt_H = patch_georef.transform * ones(3, 1);
lonlatPt = lonlatPt_H(1:2) ./ lonlatPt_H(3);

demPt_H = inv(ground_georef.transform) * lonlatPt_H;
demPt = demPt_H(1:2) ./ demPt_H(3);

ground_alt = interp2(ground_dem, demPt(2), demPt(1));
planeNormal = lonlat2normal(lonlatPt);
windows = [10; 10; 2];

%% Project patches
alt_bounds = [ground_alt - 1000, ground_alt + 1000];
#alts = linspace(alt_bounds(1), alt_bounds(2), 20);
#objs = arrayfun(@(a) mvpobj(images, patch_georef, a, planeNormal, windows), alts);

opts = optimset("MaxIter", 60, "FunValCheck", "on");

try
  [alt obj info out] = fminbnd(@(a) mvpobj(images, patch_georef, a, planeNormal, windows), alt_bounds(1), alt_bounds(2), opts);
catch
  printf("Unable to converge\n");
  alt = ground_alt;
end_try_catch

patches = mvppatches(images, patch_georef, alt, planeNormal, windows);

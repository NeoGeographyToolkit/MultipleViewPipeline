source("../../build2/init_mvp.m");

load AS15_bundle_crop2.mat;
load tile_georef.mat;

job.orbital_images = OrbitalImageCollection(im);
job.georef = GeoReference(Datum(tile_georef.datum.semi_major_axis), tile_georef.transform);
job.tile_size = [64; 64];

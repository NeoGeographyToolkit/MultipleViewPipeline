source("../../build2/init_mvp.m");
system("../../build/src/mvp/Frontend/mvpdumpjob ../real/newconf.mvp 2682 1937 12");

load 2682_1937_12.job.mat;

job.orbital_images = OrbitalImageCollection(job.orbital_images);
job.georef = GeoReference(Datum(job.georef.datum.semi_major_axis), job.georef.transform);

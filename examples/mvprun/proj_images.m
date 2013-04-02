center_pixel = (job.tile_size - 1) / 2;
curr_post = pixel2post(job.georef, center_pixel);
algovar = AlgorithmVar([-1000+job.georef.datum().semi_major_axis();
                         tanplane(curr_post); [30;30]; [0;0];
                         0; 0; 60]);

xyz = curr_post * algovar.radius();

raw_patches = job.orbital_images.back_project(xyz, algovar.orientation(), [algovar.scale();algovar.scale()], algovar.window());

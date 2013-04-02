source("../../build2/init_mvp.m");

load AS15_bundle_crop2.mat;
load tile_georef.mat;

oic = OrbitalImageCollection(im);
datum = Datum(tile_georef.datum.semi_major_axis);
georef = GeoReference(datum, tile_georef.transform);
lighter = Lighter();
objective = Objective();


post = pixel2post(georef, [32 32]);
orientation = tanplane(post);

%%%%%%%%%%%%%%%%%%

obj_helper = ObjectiveHelper(oic, lighter, objective, post);

curr_result = 1;
alts = datum.semi_major_axis() + linspace(-2000, 0, 20);
for i = alts
  seed = AlgorithmVar([i, orientation', [25,25], [0,0], 0, 0, 80]);
  result(curr_result) = obj_helper.func(seed);
  curr_result += 1;
endfor

%%%%%%%%%%%%%%%%%%

correlator = Correlator(oic, lighter, objective);

seed = AlgorithmVar([datum.semi_major_axis() + -500, orientation', [25,25], [0,0], 0, 0, 80]);
tic
d = correlator.correlate(post, seed);
toc
% vim:set syntax=octave:

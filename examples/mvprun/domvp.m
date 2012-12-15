source("../../build2/init_mvp.m");

load AS15_bundle_crop2.mat;
load tile_georef.mat;

oic = OrbitalImageCollection(im);
datum = Datum(tile_georef.datum.semi_major_axis);
georef = GeoReference(datum, tile_georef.transform);
lighter = NormalizingLighter();
objective = AbsDiffObjective();

correlator = FminbndCorrelator(oic, datum, lighter, objective);

post = georef.pixel_to_lonlat([32;32]);
orientation = datum.tangent_orientation(post);

curr_result = 1;
alts = linspace(-2000, 0, 20);
for i = alts
  seed = AlgorithmVar([i, orientation', [25,25], [0,0], 0, 0, 80]);
  result(curr_result) = correlator.obj_helper(post, seed);
  curr_result += 1;
endfor

%seed = AlgorithmVar([-500, orientation', [25,25], [0,0], 0, 0, 80]);
%d = correlator.correlate(post, seed);

%result = correlator.obj_helper(post, [-1000, orientation', [25,25], [0,0], 0, 0, 80]);

% vim:set syntax=octave:

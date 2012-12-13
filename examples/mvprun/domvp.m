source("../../build2/init_mvp.m");

load AS15_bundle_crop2.mat;
load tile_georef.mat;

oic = OrbitalImageCollection(im);
datum = Datum(tile_georef.datum.semi_major_axis);
georef = GeoReference(datum, tile_georef.transform);
lighter = NormalizingLighter();

correlator = FminbndCorrelator(oic, datum, lighter);

post = georef.pixel_to_lonlat([32;32]);
orientation = datum.tangent_orientation(post);

%curr_result = 1;
%alts = linspace(-2000, 0, 30);
%for i = alts
%  result(curr_result) = correlator.obj_helper(post, [i, orientation', [25,25], [0,0], 0, 0, 80]);
%  curr_result += 1;
%endfor

d = correlator.correlate(post, AlgorithmVar([-500, orientation', [25,25], [0,0], 0, 0, 80]));

%result = correlator.obj_helper(post, [-1000, orientation', [25,25], [0,0], 0, 0, 80]);

% vim:set syntax=octave:

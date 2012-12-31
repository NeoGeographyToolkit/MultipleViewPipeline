source("../../build2/init_mvp.m");

load AS15_bundle_crop2.mat;
load tile_georef.mat;

oic = OrbitalImageCollection(im);
datum = Datum(tile_georef.datum.semi_major_axis);
georef = GeoReference(datum, tile_georef.transform);
lighter = NormalizingLighter();
objective = AbsDiffObjective();


correlator = FminbndCorrelator(oic, lighter, objective);
%%%%%%%%%%%%%%%%%%

tile_size = [64;64];

seeder = SimpleSeeder(georef, tile_size);

while (!seeder.done())
  result = correlator.correlate(seeder.curr_post(), seeder.curr_seed());
  seeder.update(result);
endwhile

disp(["Seeder result: " num2str(seeder.result(){1}.value().algorithm_var().radius() - datum.semi_major_axis())]);

stepper = SimpleStepper(georef, tile_size, seeder.result());

tic;

cursor = 1;
total = tile_size(1)*tile_size(2);
while (!stepper.done())
  result = correlator.correlate(stepper.curr_post(), stepper.curr_seed());
  stepper.update(result);

  cursor += 1;
  printf("%d: %d / %d\r", result.algorithm_var().radius() - datum.semi_major_axis(), cursor, total);
  fflush(stdout);

  if (kbhit(1))
    result = stepper.result().alt();
    result(!stepper.result().converged()) = NA;
    save -mat result-int.mat result
    disp("Saved...");
  endif
endwhile

totalt = toc;
printf("Total time: %d\n", totalt);

result = stepper.result().alt();
result(!stepper.result().converged()) = NA;
save -mat result.mat result totalt

% vim:set syntax=octave:

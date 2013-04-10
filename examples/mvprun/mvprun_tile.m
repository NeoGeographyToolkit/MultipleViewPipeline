lighter = Lighter(job.algorithm_settings.lighter_settings);
objective = Objective(job.algorithm_settings.objective_settings);
correlator0 = Correlator(job.orbital_images, lighter, objective, job.algorithm_settings.correlator0_settings);
correlator = Correlator(job.orbital_images, lighter, objective, job.algorithm_settings.correlator_settings);

%%%%%%%%%%%%%%%%%%

seeder = Seeder(job.georef, job.tile_size, job.algorithm_settings.seeder_settings);

while (!seeder.done())
  result = correlator.correlate(seeder.curr_post(), seeder.curr_seed());
  seeder.update(result);
endwhile

disp(["Seeder result: " num2str(seeder.result(){1}.value().algorithm_var().radius() - 
                                job.georef.datum().semi_major_axis())]);

stepper = Stepper(job.georef, job.tile_size, seeder.result(), job.algorithm_settings.stepper_settings);

tic;

cursor = 1;
total = job.tile_size(1)*job.tile_size(2);
while (!stepper.done())
  result = correlator.correlate(stepper.curr_post(), stepper.curr_seed());
  stepper.update(result);

  cursor += 1;
  printf("%d: %d / %d\r", result.algorithm_var().radius() - job.georef.datum().semi_major_axis(), cursor, total);
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

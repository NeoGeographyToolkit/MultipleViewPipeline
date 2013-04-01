more off;

source("../../build/src/test/loadtestenv.m");
system("../../build/src/mvp/Frontend/mvpdumpjob ../real/newconf.mvp 2682 1937 12");

job = loadjobfile("2682_1937_12.job");

lighter = Lighter();
objective = Objective();
correlator = Correlator(job.orbital_images, lighter, objective);

%%%%%%%%%%%%%%%%%%

seeder = Seeder(job.georef, job.tile_size);

while (!seeder.done())
  result = correlator.correlate(seeder.curr_post(), seeder.curr_seed());
  seeder.update(result);
endwhile

disp(["Seeder result: " num2str(seeder.result(){1}.value().algorithm_var().radius() - 
                                job.georef.datum().semi_major_axis())]);

stepper = Stepper(job.georef, job.tile_size, seeder.result());

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

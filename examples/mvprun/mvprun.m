lighter = Lighter(job.algorithm_settings.lighter_settings);
objective = Objective(job.algorithm_settings.objective_settings);

post = pixel2post(job.georef, [32 32]);
orientation = tanplane(post);

%%%%%%%%%%%%%%%%%%%

obj_helper = ObjectiveHelper(job.orbital_images, lighter, objective, post);

curr_result = 1;
alts = job.georef.datum().semi_major_axis() + linspace(-2000, 0, 20);
result = {};
for i = alts
  seed = AlgorithmVar([i, orientation', [25,25], [0,0], 0, 0, 80]);
  result(curr_result) = obj_helper.func(seed);
  curr_result += 1;
endfor

%%%%%%%%%%%%%%%%%%

correlator = Correlator(job.orbital_images, lighter, objective, job.algorithm_settings.correlator0_settings);

seed = AlgorithmVar([job.georef.datum().semi_major_axis() + -500, orientation', [25,25], [0,0], 0, 0, 80]);
tic
d = correlator.correlate(post, seed);
toc

% vim:set syntax=octave:

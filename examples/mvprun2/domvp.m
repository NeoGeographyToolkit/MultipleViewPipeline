source("../../build/src/test/loadtestenv.m");
system("../../build/src/mvp/Frontend/mvpdumpjob ../real/newconf.mvp 2682 1937 12");

job = loadjobfile("2682_1937_12.job");

lighter = Lighter();
objective = Objective();

post = pixel2post(job.georef, [32 32]);
orientation = tanplane(post);


%%%%%%%%%%%%%%%%%%%

obj_helper = ObjectiveHelper(job.orbital_images, lighter, objective, post);

curr_result = 1;
alts = job.georef.datum().semi_major_axis() + linspace(-2000, 0, 20);
for i = alts
  seed = AlgorithmVar([i, orientation', [25,25], [0,0], 0, 0, 80]);
  result(curr_result) = obj_helper.func(seed);
  curr_result += 1;
endfor

%%%%%%%%%%%%%%%%%%

correlator = Correlator(job.orbital_images, lighter, objective);

seed = AlgorithmVar([job.georef.datum().semi_major_axis() + -500, orientation', [25,25], [0,0], 0, 0, 80]);
d = correlator.correlate(post, seed);

% vim:set syntax=octave:

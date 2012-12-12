function self = FminbndCorrelator(oic, datum, lighter)
  self=MvpClass();

  self.oic = oic;
  self.datum = datum;
  self.lighter = lighter;

  self.correlate = @correlate;
endfunction

function f = obj_helper(orbital_images, datum, post, var, lighter)
  var = AlgorithmVar(var);
  xyz = datum.geodetic_to_cartesian([post; var.alt()]);
  raw_patches = orbital_images.back_project(xyz, seed.orientation(), seed.window(), seed.scale());

  patches = zeros(seed.scale()(1), seed.scale()(2), numel(raw_patches));
  weights = ones(seed.scale()(1), seed.scale()(2), numel(raw_patches));
  for i = 1:numel(raw_patches)
    patches(:, :, i) = raw_patches{i};    
  endfor

  idx = find(isnan(patches));
  patches(idx) = 0;
  weights(idx) = 0;

  f = lighter.light(patches, weights);
endfunction

function result = correlate(self, seed)

  try
    opts = optimset("MaxIter", 80, "FunValCheck", "on");

    altMin = seed.alt() - 5000;
    altMax = seed.alt() + 5000;

    [alt confidence info output] = fminbnd(@(a) obj_helper(self.orbital_images, self.datum, post, [a; seed.vectorize()(2:end)], self.lighter),
                                         altMin, altMax, opts);
    converged = (info == 1);
    num_iterations = output.iterations;

    result = PixelResult(AlgorithmVar([alt; seed.vectorize()(2:end)]), variance, confidence, converged, num_iterations);
  catch
    result = PixelResult(AlgorithmVar(NA(12, 1)), NA, NA, NA, NA);    
  endtrycatch

endfunction

% vim:set syntax=octave:

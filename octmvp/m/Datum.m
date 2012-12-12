function self = Datum(radius)
  self = MvpClass();

  self.radius = radius
  self.geodetic_to_cartesian = @geodetic_to_cartesian;
endfunction

function xyz = geodetic_to_cartesian(self, lonlatalt)
  clo = cos(lonlatalt(1));
  slo = sin(lonlatalt(1));
  cla = cos(lonlatalt(2));
  sla = sin(lonlatalt(2));

  n = [cla * clo; cla * slo; sla];
  xyz = n * lonlatalt(3);
endfunction

% vim:set syntax=octave:

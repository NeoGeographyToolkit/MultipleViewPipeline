function self = Datum(_radius)
  self = mvpclass();

  self._radius = _radius;

  self.semi_major_axis = @(self) self._radius;
  self.geodetic_to_cartesian = @geodetic_to_cartesian;
endfunction

function xyz = geodetic_to_cartesian(self, lonlatalt)
  lonlatalt(1:2) *= pi / 180;

  clo = cos(lonlatalt(1));
  slo = sin(lonlatalt(1));
  cla = cos(lonlatalt(2));
  sla = sin(lonlatalt(2));

  n = [cla * clo; cla * slo; sla];
  xyz = n * (lonlatalt(3) + self._radius);
endfunction

% vim:set syntax=octave:

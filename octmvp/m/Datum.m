function self = Datum(radius)
  self = MvpClass();

  self.radius = radius;
  self.geodetic_to_cartesian = @geodetic_to_cartesian;
  self.tangent_orientation = @tangent_orientation;
endfunction

function xyz = geodetic_to_cartesian(self, lonlatalt)
  clo = cos(lonlatalt(1));
  slo = sin(lonlatalt(1));
  cla = cos(lonlatalt(2));
  sla = sin(lonlatalt(2));

  n = [cla * clo; cla * slo; sla];
  xyz = n * (lonlatalt(3) + self.radius);
endfunction

function q = tangent_orientation(self, lonlat)
  e = self.geodetic_to_cartesian([lonlat;0]);
  e /= norm(e);
  ex = [-e(2) e(1) 0]';
  ex /= norm(ex);
  ey = -cross(e, ex);
  R = [ex ey -e];
  q = rot2quat(R);
endfunction

% vim:set syntax=octave:

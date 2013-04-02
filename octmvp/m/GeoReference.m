function self = GeoReference(_datum, _transform)
  self = mvpclass();

  self._datum = _datum;
  self._transform = _transform;

  self.datum = @(self) self._datum;
  self.pixel_to_lonlat = @pixel_to_lonlat;
endfunction

function lonlat = pixel_to_lonlat(self, pixel)
  lonlat = self._transform * [pixel(:); 1];
  lonlat /= lonlat(3);
  lonlat = lonlat(1:2);
endfunction

% vim:set syntax=octave:

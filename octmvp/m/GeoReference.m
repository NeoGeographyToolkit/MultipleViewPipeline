function self = GeoReference(datum, transform)
  self = MvpClass();

  self.m_datum = datum;
  self.m_transform = transform;

  self.datum = @datum;
  self.pixel_to_lonlat = @pixel_to_lonlat;
endfunction

function lonlat = pixel_to_lonlat(self, pixel)
  lonlat = self.m_transform * [pixel(:); 1];
  lonlat /= lonlat(3);
  lonlat = lonlat(1:2);
endfunction

function d = datum(self)
  d = self.m_datum;
endfunction

function ll = pixel_to_lonlat(self, px)
 ll = self.m_transform * [px(:); 1];
 ll /= ll(3);
 ll = ll(1:2);  
endfunction

% vim:set syntax=octave:

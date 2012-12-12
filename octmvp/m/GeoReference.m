function self = GeoReference(datum, transform)
  self = MvpClass();

  self.m_datum = datum;
  self.m_transform = transform;

  self.datum = @datum;
  self.transform = @transform;
endfunction

function d = datum(self)
  d = self.m_datum;
endfunction

function ll = pixel_to_lonlat(px)
 ll = self.m_transform * [px(:); 1];
 ll /= ll(3);
 ll = ll(1:2);  
endfunction

% vim:set syntax=octave:

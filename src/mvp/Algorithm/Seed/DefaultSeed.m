function self = DefaultSeed(_lonlat, _value)
  self = mvpclass();
  
  self._lonlat = _lonlat;
  self._value = _value;

  self.lonlat = @(self) self._lonlat;
  self.value = @(self) self._value;
endfunction

% vim:set syntax=octave:

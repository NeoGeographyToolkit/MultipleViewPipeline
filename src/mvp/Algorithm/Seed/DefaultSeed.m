function self = Seed(_lonlat, _value)
  self = MvpClass();
  
  self._lonlat = _lonlat;
  self._value = _value;

  self.lonlat = @(self) self._lonlat;
  self.value = @(self) self._value;
endfunction

% vim:set syntax=octave:
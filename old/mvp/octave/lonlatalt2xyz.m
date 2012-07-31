function xyz = lonlatalt2xyz(datum, lonlat, alt)
  if (datum.semi_major_axis != datum.semi_minor_axis)
    error("Spheroid datums not supported");
  endif

  xyz = (datum.semi_major_axis + alt) * lonlat2normal(lonlat);

endfunction

% vim:set syntax=octave:

function xyz = lonlatalt2xyz(datum, lonlat, alt)
  if (datum.semi_major_axis == datum.semi_minor_axis)
    xyz = (datum.semi_major_axis + alt) * lonlat2normal(lonlat);
  else
    error("Spheroid datums not supported");
  endif
endfunction

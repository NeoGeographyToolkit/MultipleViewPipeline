function n = lonlat2normal(lonlat)
  clo = cos(lonlat(1));
  slo = sin(lonlat(1));
  cla = cos(lonlat(2));
  sla = sin(lonlat(2));

  n = [cla * clo; cla * slo; sla];
endfunction

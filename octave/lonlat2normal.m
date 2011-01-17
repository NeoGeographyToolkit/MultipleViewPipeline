function [n dn_dlonlat] = lonlat2normal(lonlat)
  clo = cos(lonlat(1));
  slo = sin(lonlat(1));
  cla = cos(lonlat(2));
  sla = sin(lonlat(2));

  n = [cla * clo; cla * slo; sla];

  dn_dlonlat = [-cla*slo, cla*clo, 0; -sla*clo, -sla*slo, cla]';
endfunction

% Verify grad calculation is correct
%!test
%! lonlat = [0.56; 0.23];
%! dlonlat = [1e-4; 1e-4];
%! [n1 grad]  = lonlat2normal(lonlat);
%! n2 = lonlat2normal(lonlat + dlonlat);
%! dn = grad * dlonlat;
%! dn_actual = n2 - n1;
%! pct_diff = norm((dn - dn_actual) ./ dn_actual);
%! assert(pct_diff, 0, 1e-3);

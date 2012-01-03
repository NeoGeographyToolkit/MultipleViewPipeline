function [seed patch_georef] = ts_genseed(pt, tile_georef, alt = 0, orientation = "tangent", windows = [15 15 1])

patch_georef = offsetgeoref(tile_georef, pt);

if (strcmp(orientation, "tangent"))
  lonlatPt_H = patch_georef.transform * ones(3, 1);
  lonlatPt = lonlatPt_H(1:2) ./ lonlatPt_H(3);
  orientation = lonlat2normal(lonlatPt);
endif

seed.alt = alt;
seed.orientation = orientation;
seed.windows = windows;

endfunction

% vim:set syntax=octave:

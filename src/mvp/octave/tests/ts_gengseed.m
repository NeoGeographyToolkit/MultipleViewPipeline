function [seed patch_georef] = ts_gengseed(pt, tile_georef, tile_dem, alt = "ground", orientation = "tangent", windows = [15 15 1])

if (strcmp(alt, "ground"))
  alt = tile_dem(pt(2), pt(1));
endif

if (strcmp(orientation, "ground"))
  error("Ground orientation calculation not implemented");
endif

[seed patch_georef] = ts_genseed(pt, tile_georef, alt, orientation, windows);

endfunction

% vim:set syntax=octave:

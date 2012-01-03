function [alts_out objs_out] = ts_graphobj(seed, patch_georef, images, half_range = 500, num_pts = 20)

alts = linspace(seed.alt - half_range, seed.alt + half_range, num_pts);
objs = arrayfun(@(a) mvpobj(images, patch_georef, a, seed.orientation, seed.windows), alts);

if (nargout == 0)
  plot(alts, objs);
  axis("tight");
elseif (nargout == 1)
  plot(alts, objs);
  axis("tight");
  printf("\nClick to select a new seed alt (right click to cancel)\n\n");
  fflush(stdout);
  [locX locY btn] = ginput(1);
  if (btn == 1)
    seed.alt = locX;
   else
    alts_out = seed;
    printf("Cancelled\n");
    return
  endif
  alts_out = seed;
elseif (nargout == 2)
  alts_out = alts;
  objs_out = objs;
endif

endfunction

% vim:set syntax=octave:

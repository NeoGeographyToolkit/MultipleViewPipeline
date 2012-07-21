function [alts_out objs_out] = ts_plotobj(seed, patch_georef, images, mvpoptions, num_pts = 20)

alts = linspace(seed.alt - mvpoptions.alt_range, seed.alt + mvpoptions.alt_range, num_pts);
objs = arrayfun(@(a) mvpobj(a, seed.orientation, seed.windows, patch_georef, images, mvpoptions), alts);

if (nargout < 2)
  plot(alts, objs);
  axis("tight");
  % TODO: bug?: for some reason, the +/- 1 is needed for the line to show...
  line([seed.alt, seed.alt], [min(objs)-1, max(objs)+1]);

  if (nargout == 1)
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
  endif
else
  alts_out = alts;
  objs_out = objs;
endif 

endfunction

% vim:set syntax=octave:

function rws = mvpgui_cmd_replot(ws, args)
  if (numel(args) != 0)
    error("replot: invalid number of arguments");
  endif

  switch (ws.plot)
    case "radplot"
      axis("auto");
      replot;
    case "lonlatplot"
      axis("auto", "image");
      replot;
    otherwise
      error("replot: no current plot!");
  endswitch

  rws = ws;  
endfunction

% vim:set syntax=octave:

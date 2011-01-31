function rws = mvpgui_cmd_replot(ws, args)
  if (numel(args) != 0)
    error("replot: invalid number of arguments");
  endif

  switch (ws.plot)
    case {"radplot" "lonlatplot"}
      axis([1 2 3 4], "auto");
      replot;
    otherwise
      error("replot: no current plot!");
  endswitch

  rws = ws;  
endfunction

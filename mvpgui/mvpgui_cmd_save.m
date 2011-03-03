function rws = mvpgui_cmd_save(ws, args)
  if (numel(args) != 1)
    error("save: invalid number of arguments");
  endif

  switch(ws.plot)
    case "radplot"
      radplot = ws.radplot;
      save(args{1}, "radplot");
    case "lonlatplot"
      lonlatplot = ws.lonlatplot;
      save(args{1}, "lonlatplot");
    otherwise
      error("save: nothing to save");
  endswitch

  rws = ws;  
endfunction

% vim:set syntax=octave:

function rws = mvpgui_cmd_poi(ws, args)
  switch (numel(args))
    case 0
      printf("\npoi = (%d %d)\n\n", ws.poi(1), ws.poi(2));
    case 2
      ws.poi = floor([str2num(args{1}) str2num(args{2})]);
    otherwise
      error("poi: invalid number of arguments");    
  endswitch
  rws = ws;  
endfunction

% vim:set syntax=octave:

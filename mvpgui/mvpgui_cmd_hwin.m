function rws = mvpgui_cmd_hwin(ws, args)
  switch (numel(args))
    case 0
      printf("\nhwin = %d\n\n", ws.hwin);
    case 1
      ws.hwin = floor(str2num(args{1}));
    otherwise
      error("hwin: invalid number of arguments");    
  endswitch
  rws = ws;  
endfunction

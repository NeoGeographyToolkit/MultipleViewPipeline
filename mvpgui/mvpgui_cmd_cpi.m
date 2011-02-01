function rws = mvpgui_cmd_cpi(ws, args)
  if (numel(args) != 0)
    error("cpi: invalid number of arguments");
  endif
  if (strcmp(ws.plot, "none"))
    error("cpi: no current plot");
  endif

  oldcp = ws.cp;

  printf("\nSelect a point (right click to cancel)\n\n");
  ws = mvpgui_cmd_replot(ws, {});
  [locX locY btn] = ginput(1);
  if (btn == 1)
    switch(ws.plot)
      case "radplot"
        ws.cp(3) = locX;
      case "lonlatplot"
        ws.cp(1) = locX;
        ws.cp(2) = locY;
      otherwise
        error("cpi: logic error");    
    endswitch
  else
    error("cpi: cancelled by user");
  endif

  printf("cp = %f %f %f ->\n", oldcp(1), oldcp(2), oldcp(3));
  printf("     %f %f %f\n\n", ws.cp(1), ws.cp(2), ws.cp(3)); 

  rws = ws;  
endfunction

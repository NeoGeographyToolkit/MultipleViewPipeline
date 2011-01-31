function rws = mvpgui_cmd_cp(ws, args)
  switch (numel(args))
    case 0
      printf("\ncp = %f %f %f\n\n", ws.cp(1), ws.cp(2), ws.cp(3));
    case 1
      if (strcmp(args{1}, "init"))
        lonlatH = ws.georef * [ws.poi(1); ws.poi(2); 1];
        lonlat = lonlatH(1:2) / lonlatH(3);
        rad = ws.dem(ws.poi(1), ws.poi(2));
        ws.cp = [lonlat' rad];
      else
        error(["cp: unrecognized option " args{1}]);
      endif
    case 3
      for k = 1:3
        if (!strcmp(args{k}, "."))
          ws.cp(k) = str2num(args{k});
        endif
      endfor
    otherwise
      error("cp: invalid number of arguments");    
  endswitch
  rws = ws;  
endfunction

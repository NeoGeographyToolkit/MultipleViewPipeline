function rws = mvpgui_cmd_help(ws, args)
  switch (numel(args))
    case 0
      printf("\nUsage: help [command]\n\n");
      printf("Available commands: \n");

      cmdlist = fieldnames(ws.cmddb);
      for i = 1:numel(cmdlist)
        printf("   %s\n", cmdlist{i});
      endfor

      printf("\n\n");
    case 1
      try
        cmd = getfield(ws.cmddb, args{1});
      catch
        error("help: unrecognized command");
      end_try_catch

      printf("\nUsage: %s\n\n", cmd.usage);
      disp(cmd.desc);
      printf("\n\n");
    otherwise
      error("help: invalid number of arguments");    
  endswitch
  rws = ws;  
endfunction

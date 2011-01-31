function rws = mvpgui_cmd_radplot(ws, args)
  radbegin = -1;
  radend = -1;
  npts = -1;

  switch (numel(args))
    case 1
      radbegin = ws.cp(3) - str2num(args{1});
      radend = ws.cp(3) + str2num(args{1});
      npts = 10;
    case 2
      radbegin = ws.cp(3) - str2num(args{1});
      radend = ws.cp(3) + str2num(args{1});
      npts = floor(str2num(args{2}));
    otherwise
      error("radplot: invalid number of arguments");    
  endswitch

  rads = linspace(radbegin, radend, npts);

  printf("\n");

  obj = zeros(npts, 1);
  for k = 1:npts
    obj(k) = mvpobj([ws.cp(1:2) rads(k)], ws.poi, ws.georef, ws.orbs, ws.hwin, 10);
    printf("%d / %d\r", k, npts);
  endfor
  printf("\n\n");

  ws.fig = plot(rads, obj, "*-");
  ws.plot = "radplot";
  pause(0);

  rws = ws;  
endfunction

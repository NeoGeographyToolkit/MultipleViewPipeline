function rws = mvpgui_cmd_rplot(ws, args)
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
      error("rplot: invalid number of arguments");    
  endswitch

  ws.radplot.poi = ws.poi;
  ws.radplot.georef = ws.georef;
  ws.radplot.cp = ws.cp;
  ws.radplot.rads = linspace(radbegin, radend, npts);

  printf("\n");

  ws.radplot.obj = zeros(npts, 1);
  for k = 1:npts
    ws.radplot.obj(k) = mvpobj([ws.radplot.cp(1:2) ws.radplot.rads(k)], ws.poi, ws.georef, ws.orbs, ws.hwin, -1);
    printf("%d / %d\r", k, npts);
  endfor
  printf("\n\n");

  ws.fig = plot(ws.radplot.rads, ws.radplot.obj, "*-");
  ws.plot = "radplot";
  pause(0);

  rws = ws;  
endfunction

% vim:set syntax=octave:

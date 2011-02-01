function rws = mvpgui_cmd_lonlatplot(ws, args)
  lonbegin = -1;
  lonend = -1;
  latbegin = -1;
  latend = -1;
  npts = -1;

  switch (numel(args))
    case 1
      lonbegin = ws.cp(1) - str2num(args{1});
      lonend = ws.cp(1) + str2num(args{1});
      
      latbegin = ws.cp(2) - str2num(args{1});
      latend = ws.cp(2) + str2num(args{1});

      npts = 15;
    case 2
      lonbegin = ws.cp(1) - str2num(args{1});
      lonend = ws.cp(1) + str2num(args{1});
      
      latbegin = ws.cp(2) - str2num(args{1});
      latend = ws.cp(2) + str2num(args{1});

      npts = floor(str2num(args{2}));
    otherwise
      error("radplot: invalid number of arguments");    
  endswitch

  ws.lonlatplot.poi = ws.poi;
  ws.lonlatplot.georef = ws.georef;
  ws.lonlatplot.cp = ws.cp;
  ws.lonlatplot.lons = linspace(lonbegin, lonend, npts);
  ws.lonlatplot.lats = linspace(latbegin, latend, npts);

  printf("\n");

  ws.lonlatplot.obj = zeros(npts, npts);
  for x = 1:npts
    for y = 1:npts
      plane = [ws.lonlatplot.lons(x) ws.lonlatplot.lats(y) ws.lonlatplot.cp(3)];
      ws.lonlatplot.obj(x, y) = mvpobj(plane, ws.poi, ws.georef, ws.orbs, ws.hwin, -1);
      printf("%d / %d\r", (x - 1) * npts + y, npts * npts);
    endfor
  endfor
  printf("\n\n");

  ws.fig = imagesc(ws.lonlatplot.lons, ws.lonlatplot.lats, ws.lonlatplot.obj);
  ws.plot = "lonlatplot";
  axis("auto", "image");
  pause(0);

  rws = ws;  
endfunction

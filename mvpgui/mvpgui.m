DATUM_RADIUS = 1737400; %Moon

% Parse command args
try
  args = mvpgui_parseArgs(argv(), nargin);
catch
  printf("Error: %s\n\n", lasterror.message);
  printf("Usage: mvpgui dem.tif orbit1.tif orbit1.pinhole\n");
  printf("                      orbit2.tif orbit2.pinhole\n"); 
  printf("                     [orbit3.tif orbit3.pinhole ...]\n");
  exit(1);
end_try_catch

% Load data into workspace (ws)
printf("Loading images... ");
try
  [ws.dem ws.georef] = imread_vw(args.demFile);
  ws.dem += DATUM_RADIUS;

  for k = 1:args.numOrbs
    ws.orbs{k}.img = imread_vw(args.imgFiles{k});
    ws.orbs{k}.cam = loadcam_vw(args.camFiles{k});
  endfor
  printf("Done!\n");
catch
  printf("\nError loading images: %s\n", lasterror.message);
  exit(1);
end_try_catch

% Initialize the workspace
ws.poi = floor(size(ws.orbs{1}.img) / 2);
ws = mvpgui_cmd_cp(ws, {"init"});
ws.hwin = 10;
ws.plot = "none";

% Enter interactive shell
while 1
  try
    cmd = strsplit(tolower(input("mvp> ", "s")), " ", true);
  catch
    % Ctrl-D exits
    printf("\n");
    exit(0);
  end_try_catch

  if length(cmd) == 0
    continue;
  endif

  fn = @() error("Logic error");
  switch (cmd{1})
    case "help"
      fn = @mvpgui_cmd_help;
    case "poi"
      fn = @mvpgui_cmd_poi;
    case "hwin"
      fn = @mvpgui_cmd_hwin;
    case "cp"
      fn = @mvpgui_cmd_cp;
    case "cpi"
      fn = @mvpgui_cmd_cpi;
    case "radplot"
      fn = @mvpgui_cmd_radplot;
    case "replot"
      fn = @mvpgui_cmd_replot;
    case "save"
      fn = @mvpgui_cmd_save;
    case "exit"
      fn = @mvpgui_cmd_exit;
    otherwise
      printf("Unrecognized command: %s\n", cmd{1});
      continue;
  endswitch

  try
    ws = fn(ws, {cmd{2:end}});
  catch
    printf("Error: %s\n", lasterror.message);
  end_try_catch
endwhile

% vim:set syntax=octave:

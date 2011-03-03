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

% Initialize the commands
ws.cmddb = struct(
  "help", @mvpgui_cmd_help,
  "poi", @mvpgui_cmd_poi,
  "hwin", @mvpgui_cmd_hwin,
  "cp", @mvpgui_cmd_cp,
  "cpi", @mvpgui_cmd_cpi,
  "rplot", @mvpgui_cmd_rplot,
  "llplot", @mvpgui_cmd_llplot,
  "replot", @mvpgui_cmd_replot,
  "save", @mvpgui_cmd_save,
  "exit", @mvpgui_cmd_exit);

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

  try
    fn = getfield(ws.cmddb, cmd{1});
  catch
      printf("Unrecognized command: %s\n", cmd{1});
      continue;
  end_try_catch

  try
    ws = fn(ws, {cmd{2:end}});
  catch
    printf("Error: %s\n", lasterror.message);
  end_try_catch
endwhile

% vim:set syntax=octave:

DATUM_RADIUS = 1737400; %Moon

% Parse command args
try
  args = mvpgui_parseArgs(argv(), nargin);
catch
  printf("Error: %s\n\n", lasterror.message);
  printf("Usage: mvpgui dem.tif orbit1.tif orbit1.pinhole\n");
  printf("                      orbit2.tif orbit2.pinhole\n"); 
  printf("                     [orbit3.tif orbit3.pinhole ...]");
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
  printf("\nError loading images: %s", lasterror.message);
  exit(1);
end_try_catch

% Enter interactive shell
while 1
  try
    cmd = strsplit(tolower(input("mvp> ", "s")), " ", true);
  catch
    % Ctrl-D exits
    exit(0);
  end_try_catch

  if length(cmd) == 0
    continue;
  endif

endwhile

% vim:set syntax=octave:

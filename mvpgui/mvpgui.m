DATUM_RADIUS = 1737400; %Moon

% Parse command args
[args err] = mvpgui_parseArgs(argv(), nargin);

if (err)
  printf("Error: %s\n", err);
  printf("Usage: mvpgui dem.tif orbit1.tif orbit1.pinhole orbit2.tif orbit2.pinhole [orbit3.tif orbit3.pinhole ...]");
  exit(1);
endif

% Load data
printf("Loading images... ");
[dem georef] = imread_vw(args.demFile);
dem += DATUM_RADIUS;

for k = 1:args.numOrbs
  orbs{k}.img = imread_vw(args.imgFiles{k});
  orbs{k}.cam = loadcam_vw(args.camFiles{k});
endfor
printf("Done!\n");

% Show command prompt
while 1
  try
    act = input("mvp> ", "s");
  catch
    % Ctrl-D exits
    exit(0);
  end_try_catch
endwhile

% vim:set syntax=octave:

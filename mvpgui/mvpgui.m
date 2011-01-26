DATUM_RADIUS = 1737400; %Moon

function printUsage()
  printf("Usage: mvpgui dem.tif orbit1.tif orbit1.pinhole orbit2.tif orbit2.pinhole [orbit3.tif orbit3.pinhole ...]");
endfunction

% Parse command args
if nargin < 5
  printUsage();
  exit(1);
endif

args = argv();

demFile = args{1};

numOrbs = (numel(args) - 1) / 2;
if floor(numOrbs) != numOrbs
  printf("Error: Supply a camera model for each orbital image!");
  printUsage();
  exit(1);
endif 

for k = 1:numOrbs
  imgFiles{k} = args{2 * k};
  camFiles{k} = args{2 * k + 1};
endfor

% Load data
printf("Loading images... ");
[dem georef] = imread_vw(demFile);
dem += DATUM_RADIUS;

for k = 1:numOrbs
  orbs{k}.img = imread_vw(imgFiles{k});
  orbs{k}.cam = loadcam_vw(camFiles{k});
endfor
printf("Done!\n");

% Show command prompt
while 1
  try
    act = input("mvp> ", "s");
  catch
    % Cntrl-D exits
    exit(0);
  end_try_catch
endwhile

% vim:set syntax=octave:

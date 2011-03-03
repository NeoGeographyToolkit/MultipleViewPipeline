function [args err] = mvpgui_parseArgs(pargv, pnargin)
args = {};
err = "";

if pnargin < 5
  error("Too few arguments");
endif

args.demFile = pargv{1};

args.numOrbs = (numel(pargv) - 1) / 2;

if floor(args.numOrbs) != args.numOrbs
  error("Supply a camera model for each orbital image!");
endif 

for k = 1:args.numOrbs
  args.imgFiles{k} = pargv{2 * k};
  args.camFiles{k} = pargv{2 * k + 1};
endfor

endfunction

% vim:set syntax=octave:

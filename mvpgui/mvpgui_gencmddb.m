function cmddb = mvpgui_gencmddb()
cmddb = struct(
  "help", gencmd(
  @mvpgui_cmd_help,
  "help [command]",
  "Display help for a command"),


  "poi", gencmd(
  @mvpgui_cmd_poi,
  "poi [x y]",
 ["With no arguments, poi displays the current point of interest."
  "Otherwise, it sets the point of interest to x y."]),

  "hwin", gencmd(
  @mvpgui_cmd_hwin,
  "hwin [size]",
 ["With no arguments, hwin displays the current half-window size."
  "Otherwise, it sets the half-window size to size."]),

  "cp", gencmd(
  @mvpgui_cmd_cp,
  "cp [lon lat h]",
 ["With no arguments, cp displays the current plane's position and "
  "orientation. Otherwise, it sets the current position and orientation "
  "to lon lat h. If any argument is replaced with a \"*\", it will not "
  "be changed."]),
 
  "cpi", gencmd(
  @mvpgui_cmd_cpi,
  "cpi",
 ["An interactive version of cp: pick a new current plane by"
  "clicking on the current plot"]),

  "rplot", gencmd(
  @mvpgui_cmd_rplot,
  "rplot range [numpts]",
 ["Plot the objective function as a function of the height of the plane while"
  "keeping the orientation constant. Optional argument numpts specifies the number"
  "of data points to collect, and defaults to 10."]),

  "llplot", gencmd(
  @mvpgui_cmd_llplot,
  "llplot range [numpts]",
 ["Plot the objective function as a function of the orientation of the"
  "plane while keeping the height constant. Optional argument numpts"
  "specifies the number of data points to collect in each direction,"
  "and defaults to 10."]),

  "replot", gencmd(
  @mvpgui_cmd_replot,
  "replot",
  "Refresh the current plot window"),

  "save", gencmd(
  @mvpgui_cmd_save,
  "save filename",
  "Save current plot and workspace to filename"),

  "exit", gencmd(
  @mvpgui_cmd_exit,
  "exit",
  "Exit mvpgui"));
endfunction

function cmd = gencmd(fun, usage, desc)
  cmd = struct("fun", fun, "usage", usage, "desc", desc);
endfunction

% vim:set syntax=octave:

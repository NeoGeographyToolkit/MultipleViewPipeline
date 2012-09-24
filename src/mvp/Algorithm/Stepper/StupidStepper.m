function self = StupidStepper(var)
  self = MvpStepper();
  self.memvar = var;
  self.zing = @zing;
  self.zap = @zap;
endfunction

function zing(self, a, b)
  disp(["Octave: StupidStepper::zing with " num2str(a) " " num2str(b)]);
  disp(["memvar: " num2str(self.memvar)]);
endfunction

function rtn = zap(self, a)
  disp(["Octave: StupidStepper::zap with " num2str(a)]);
  disp(["memvar: " num2str(self.memvar)]);
  rtn = 3;
endfunction

% vim:set syntax=octave:

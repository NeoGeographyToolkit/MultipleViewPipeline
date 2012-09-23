function self = StupidStepper()
  self = MvpStepper();
  self.zing = @zing;
  self.zap = @zap;
endfunction

function zing(self, a, b)
  disp(["Octave: StupidStepper::zing with " num2str(a) " " num2str(b)]);
endfunction

function rtn = zap(self, a)
  disp(["Octave: StupidStepper::zap with " num2str(a)]);
  rtn = 3;
endfunction

% vim:set syntax=octave:

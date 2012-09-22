function self = StupidStepper()
  function zing(self, a, b)
    disp(["Octave: StupidStepper::zing with " num2str(a) " " num2str(b)]);
  endfunction

  function rtn = zap(self, a)
    disp(["Octave: StupidStepper::zap with " num2str(a)]);
  endfunction

  self = MvpStepper();
  self.zing = @zing;
  self.zap = @zap;
endfunction

% vim:set syntax=octave:

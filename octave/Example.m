function self = Example(init1, init2)
  disp(["Example constructed with " num2str(init1) " and " num2str(init2)]);

  self = MvpClass();
  self.memvar = init1;

  self.memvoid = @memvoid;
  self.memfun = @memfun;
endfunction

function memvoid(self, arg1)
  disp(["memvoid(" num2str(arg1) ")"]);
  disp(["memvar is " num2str(self.memvar)]);
  self.memvar = self.memvar + 1;
endfunction

function rtn = memfun(self, arg1)
  rtn = arg1 * self.memvar;
endfunction

%!test
%! t = Example(2, 3);
%! t.memvoid(100);
%! assert(t.memfun(4), 12);

% vim:set syntax=octave:

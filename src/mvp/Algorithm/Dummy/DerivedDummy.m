function self = DerivedDummy(x, y)
  self = mvpclass();

  self.m_x = 0;
  self.m_y = 0;

  if (nargin > 0)
    self.m_x = x;
  endif

  if (nargin > 1)
    self.m_y = y;
  endif

  self.void0 = @void0;
  self.void1 = @void1;
  self.void2 = @void2;
  self.function0 = @function0;
  self.function1 = @function1;
  self.function2 = @function2;
  self.x = @x;
  self.y = @y;
endfunction

function void0(self)
  disp("ASDFASDF");
endfunction

function void1(self)
endfunction

function void2(self)
endfunction

function rtn = function0(self)
  rtn = 0;
endfunction

function rtn = function1(self, a)
  rtn = a;
endfunction

function rtn = function2(self, a, b)
  rtn = a + b;
endfunction

function rtn = x(self)
  rtn = self.m_x;
endfunction

function rtn = y(self)
  rtn = self.m_y;
endfunction

% vim:set syntax=octave:

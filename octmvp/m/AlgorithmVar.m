function self = AlgorithmVar(v)
  if (numel(v) != 12)
    error("Must construct with vector size 12");
  endif

  self = MvpClass();

  self.v = v(:);
  self.alt = @alt;
  self.orientation = @orientation;
  self.window = @window;
  self.gwindow = @gwindow;
  self.smooth = @smooth;
  self.gsmooth = @gsmooth;
  self.scale = @scale;
  self.vectorize = @vectorize;
endfunction

function result = alt(self)
  result = self.v(1);
endfunction

function result = orientation(self)
  result = self.v(2:5);
endfunction

function result = window(self)
  result = self.v(6:7);
endfunction

function result = gwindow(self)
  result = self.v(8:9);
endfunction

function result = smooth(self)
  result = self.v(10);
endfunction

function result = gsmooth(self)
  result = self.v(11);
endfunction

function result = scale(self)
  result = self.v(12);
endfunction

function result = vectorize(self)
  result = self.v;
endfunction

% vim:set syntax=octave:

function self = ObjectiveHelper(_oic, _lighter, _objective, _post)
  self = mvpclass();

  self._oic = _oic;
  self._lighter = _lighter;
  self._objective = _objective;
  self._post = _post;

  self._curr_algovar = zeros(12, 1);
  self._curr_albedo_box = {};

  self.func = @func;
  self.grad = @grad;
  self.reproject = @reproject;
endfunction

function reproject(self, algovar)
  xyz = self._post * algovar.radius();

  raw_patches = self._oic.back_project(xyz, algovar.orientation(), algovar.scale(), algovar.window());

  patch_box = PatchBox(raw_patches, algovar.gwindow(), algovar.smooth(), algovar.gsmooth());

  self._curr_albedo_box = self._lighter.light(patch_box);
  self._curr_algovar = algovar.vectorize();
endfunction

function f = func(self, algovar)
  if (any(self._curr_algovar != algovar.vectorize()))
    self.reproject(algovar);
  endif

  if (self._curr_albedo_box.planes() > 1)
    f = self._objective.func(self._curr_albedo_box);
  else
    f = NA;
  endif
endfunction

function g = grad(self, algovar)
  if (any(self._curr_algovar != algovar.vectorize()))
    self.reproject(algovar);
  endif

  if (self._curr_albedo_box.planes() > 1)
    g = self_objective.grad(self._curr_albedo_box);
  else
    g = NA;
  endif
endfunction

% vim:set syntax=octave:

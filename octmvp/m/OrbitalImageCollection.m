function self = OrbitalImageCollection(im)
  self = MvpClass();

  self.im = im;
  self.back_project = @back_project;
endfunction

function result = back_project(self, xyz, orientation, scale, sz)
  cursor = 1;
  for j = 1:numel(self.im);
    patch = _back_project(self.im(j).data, self.im(j).camera, xyz, orientation, [scale scale], sz);
    if any(patch(:))
      result{cursor} = patch;
      cursor += 1;
    endif
  endfor
endfunction

%!function c = camera(center, orientation, focus, sz)
%!  intrinsics = diag([focus(1) -focus(2) 1]);
%!  intrinsics(1:2, 3) = sz / 2;
%!  extrinsics = [orientation -orientation*center(:)];
%!  c = intrinsics * extrinsics;
%!endfunction

%!function b = patcheq(patch1, patch2, tol)
%!  diff = abs(patch1 - patch2);
%!  numvalid = sum(!isnan(diff)(:));
%!  
%!  diff(isnan(diff)) = 0;
%!  sumvalid = sum(diff(:));
%!
%!  b = numvalid && sumvalid < tol;
%!endfunction

%!test
%!  sz = [64, 32];
%!  imdata = rand(fliplr(sz));
%!
%!  % Camera at z = 1, looking back at origin. 
%!  im(1).data = imdata;
%!  im(1).camera = camera([0 0  1], diag([1 -1 -1]), [1 1], sz); 
%!
%!  % Camera at z = 1, looking back at origin.
%!  im(2).data = imdata;
%!  im(2).camera = camera([0 0 1], diag([1 -1 -1]), [1 1], sz);
%!
%!  % Camera at z = -1, looking toward origin.
%!  im(3).data = imdata;
%!  im(3).camera = camera([0 0 -1], eye(3), [1 1], sz);
%!
%!  oic = OrbitalImageCollection(im);
%!
%!  % Plane at z = 0
%!  patches = oic.back_project([0 0 0], [1 0 0 0], 1, sz);
%!
%!  assert(patcheq(patches{1}, imdata, 1e-4));
%!  assert(patcheq(patches{2}, imdata, 1e-4));
%!  assert(!patcheq(patches{3}, imdata, 1e-4));
%!
%!  % Plane at z = -1, patch scale 2m/px
%!  patches = oic.back_project([0 0 -1], [1 0 0 0], 2, sz);
%!
%!  assert(patcheq(patches{1}, imdata, 1e-4));
%!  assert(patcheq(patches{2}, imdata, 1e-4));
%!  assert(numel(patches), 2);
%!
%!  % Plane at z = 0, rotated 180deg about x axis (vertical flip) 
%!  patches = oic.back_project([0 0 0], [0 1 0 0], 1, sz);
%!
%!  assert(!patcheq(patches{1}, imdata, 1e-4));
%!  assert(!patcheq(patches{2}, imdata, 1e-4));
%!  assert(patcheq(patches{3}, imdata, 1e-4));

% vim:set syntax=octave:

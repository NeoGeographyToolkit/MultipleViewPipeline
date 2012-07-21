function result = geotrans(image, from_georef, to_georef, sz)
  %% Draw DEM on tile
  P = inv(from_georef.transform) * to_georef.transform;
  [X Y] = meshgrid(1:sz(1), 1:sz(2));
  D = [X(:) Y(:) ones(prod(sz), 1)]';
  PD = P * D;

  xw = reshape(PD(1,:) ./ PD(3,:), sz(2), sz(1));
  yw = reshape(PD(2,:) ./ PD(3,:), sz(2), sz(1));

  result = imremap(image, xw, yw, "bilinear");
endfunction

%% TODO: test me

% vim:set syntax=octave:

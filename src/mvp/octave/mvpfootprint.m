function [result, variance, converged, num_iterations] = mvpfootprint(seed, georef, images, settings)
  if (gausskernel("kernsize", seed.windows(1)) != 3)
    result = seed;
    variance = 0;
    converged = true;
    num_iterations = 0;
    return;
  endif

  lonlat_h = georef.transform * [1; 1; 1];
  lonlat = lonlat_h(1:2) / lonlat_h(3);

  xyz = lonlatalt2xyz(georef.datum, lonlat, 0);
  xyz_h = [xyz; 1];

  overlap = 0;

  for img = images
    px_h = img.camera * xyz_h;
    px = px_h(1:2) /  px_h(3);

    % Note that VW labels the upper left hand corner of the upper left hand pixel 
    % (0, 0) So the lower right hand corner of the that same pixel is (0.999.., 0.999...). 
    % Thus, a coordinate is "contained" in that pixel if (0, 0) <= (x, y) < (1, 1).
    %
    % For Octave, the upper left hand corner of the upper left hand pixel is (1, 1) 
    % The lower right hand of that same pixel is (1.999...,1.999...). Thus, a
    % coordinate "contained" in that pixel will be (1, 1) <= (x, y) < (2, 2).
    %
    % This is why we must add [1; 1] to the image size before comparing it.
    if (px >= [1; 1] && px < (fliplr(size(img.data))' + [1; 1]))
      overlap++;
    endif
  endfor

  result.alt = overlap;
  result.orientation = [overlap; overlap; overlap];
  result.windows = [overlap; overlap; overlap];
  variance = overlap;
  converged = overlap > 0;
  num_iterations = overlap;
endfunction

% vim:set syntax=octave:

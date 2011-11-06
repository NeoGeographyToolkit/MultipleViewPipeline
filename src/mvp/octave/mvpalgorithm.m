% seed: scalar struct
% seed.post_height
% seed.orientation
% seed.windows

% result: scalar struct
% result.variance
% result.converged
% result.num_iterations_to_converge

% settings: scalar struct
% settings.post_height_limits
% settings.fix_orientation
% settings.fix_windows


% all octave images have 1,1 as upper left pixel.
% Note for cameras and georefs

% so (1, 1) for the georef is the center of the patch

% images: struct array
% images[x].camera
% images[x].data

function [result, variance, converged, num_iterations]  = mvpalgorithm(seed, georef, images, settings)
  lonlat_h = georef.transform * [1; 1; 1];
  lonlat = lonlat_h(1:2) / lonlat_h(3);

  assert(georef.datum.semi_major_axis == georef.datum.semi_minor_axis);
  xyz = georef.datum.semi_major_axis * lonlat2normal(lonlat);
  xyz_h = [xyz; 1];

  overlap = 0;

  for img = images
    px_h = img.camera * xyz_h;
    px = px_h(1:2) /  px_h(3);

    if (px > [1; 1] && px < flipud(size(img.data)'))
      overlap++;
    endif
  endfor

  result.post_height = overlap;
  result.orientation = [overlap; overlap; overlap];
  result.windows = [overlap; overlap; overlap];
  variance = overlap;
  converged = overlap > 0;
  num_iterations = overlap;
endfunction

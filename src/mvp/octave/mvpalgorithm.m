% seed.height
% seed.variance
% seed.orientation
% seed.windows

% settings.post_height_limits
% settings.fix_orientation
% settings.fix_windows


% all octave images have 1,1 as upper left pixel.
% Note for cameras and georefs

% so (1, 1) for the georef is the center of the patch

% images.camera
% images.data

% OrbitalImage vs OrbitalImageFile
% OrbitalImageFileDesc

% Descriptions can encapsulate entire objects
% Settings are settings

function result = mvpalgorithm(seed, georef, images, settings)
  lonlat_h = georef * [1; 1; 1];
  lonlat = lonlat_h(1:2) / lonlat_h(3);
 
  xyz = seed.height * lonlat2normal(lonlat);
  xyz_h = [xyz; 1];

  overlap = 0;

  for img = images
    px_h = img.camera * xyz;
    px = px_h(1:2) /  px_h(3);

    if (px > [0; 0] && px < size(img.data))
      overlap++;
    endif
  endfor

  result = seed;
  result.height = overlap;
endfunction

#include <octave/oct.h>
#include <octave/lo-ieee.h> // octave_NA

#include "MathSupport.h"

// image, camera, xyz, orientation, scale, sz
DEFUN_DLD(_back_project, args, nargout, "Oct project") {

  if (args.length() != 6) {
    error("Usage: _back_project(image,camera,xyz,orientation,scale,size)");
    return octave_value();
  }

  Matrix image(args(0).matrix_value());
  Matrix camera(args(1).matrix_value());

  ColumnVector xyz(args(2).column_vector_value());
  ColumnVector orientation(args(3).column_vector_value());

  ColumnVector scale(args(4).column_vector_value());

  ColumnVector size(args(5).column_vector_value());
  
  Matrix morientation = quat2rot(orientation);

  Matrix patch(size(1), size(0));

  for (int r = 0; r < size(1); r++) {
    for (int c = 0; c < size(0); c++) {
      ColumnVector cursor(2);
      cursor(0) = c;
      cursor(1) = r;

      ColumnVector patch_pt = product(cursor - size / 2, scale);
      patch_pt.idx_add(2, 0);

      ColumnVector world_pt = morientation * patch_pt + xyz;
      world_pt.idx_add(3, 1);

      ColumnVector image_pt = camera * world_pt;
      image_pt /= image_pt(2);

      if (image_pt(0) < image.cols() && image_pt(1) < image.rows() &&
          image_pt(0) >= 0 && image_pt(1) >= 0) 
      {
        patch(r, c) = bilinear_interp(image, image_pt(0), image_pt(1));  
      } else {
        patch(r, c) = ::octave_NA;
      }
    }
  }
  return octave_value(patch);
}

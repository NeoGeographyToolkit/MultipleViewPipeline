#include <octave/oct.h>
#include <octave/lo-ieee.h> // octave_NA

#include "MathSupport.h"

// image, homog, sz
DEFUN_DLD(_do_homog, args, nargout, "Oct project") {

  if (args.length() != 3) {
    error("Usage: _do_homog(image,homog,sz)");
    return octave_value();
  }

  Matrix image(args(0).matrix_value());
  Matrix homog(args(1).matrix_value());
  Matrix sz(args(2).matrix_value());

  if (!(homog.rows() == 3 && homog.cols() == 3)) {
    error("Error: homog must be 3x3");
    return octave_value();
  }

  double h[3][3];
  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      h[r][c] = homog(r, c);
    }
  }

  double rows = sz(1);
  double cols = sz(0);

  Matrix patch(rows, cols);
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      double x = h[0][0] * c + h[0][1] * r + h[0][2];
      double y = h[1][0] * c + h[1][1] * r + h[1][2];
      double w = h[2][0] * c + h[2][1] * r + h[2][2];
      x /= w;
      y /= w;
      patch(r, c) = bilinear_interp2(image, x, y);
    }
  }

  return octave_value(patch);
}

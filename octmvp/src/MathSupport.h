#ifndef __MATHSUPPORT_H__
#define __MATHSUPPORT_H__

#include <octave/oct.h>

double bilinear_interp(Matrix const& im, double x, double y) {
  // See http://en.wikipedia.org/wiki/Bilinear_interpolation
  double x1 = floor(x), y1 = floor(y);
  double x2 = x1 + 1, y2 = y1 + 1;

  double Q11 = im(y1, x1), Q12 = im(y2, x1);
  double Q21 = im(y1, x2), Q22 = im(y2, x2);

  double x2x = x2 - x, xx1 = x - x1;
  double y2y = y2 - y, yy1 = y - y1;

  return Q11*x2x*y2y + Q21*xx1*y2y + Q12*x2x*yy1 + Q22*xx1*yy1;
}

Matrix quat2rot(ColumnVector const& quat) {
  Matrix rot(3, 3);
  double w = quat(0);
  double x = quat(1);
  double y = quat(2);
  double z = quat(3);
  double w2 = w*w, x2 = x*x, y2 = y*y, z2 = z*z;
  double wx = w*x, wy = w*y, wz = w*z;
  double xy = x*y, yz = y*z, zx = z*x;

  rot(0,0) = w2 + x2 - y2 - z2;
  rot(1,1) = w2 - x2 + y2 - z2;
  rot(2,2) = w2 - x2 - y2 + z2;
  rot(0,1) = 2 * (xy - wz);
  rot(0,2) = 2 * (zx + wy);
  rot(1,2) = 2 * (yz - wx);
  rot(1,0) = 2 * (xy + wz);
  rot(2,0) = 2 * (zx - wy);
  rot(2,1) = 2 * (yz + wx);

  return rot;
}



#endif

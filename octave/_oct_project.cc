#include <octave/oct.h>
#include <octave/lo-ieee.h> // octave_NA

double interp(Matrix const& im, double x, double y) {
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

// image, camera, xyz, orientation, scale, sz
DEFUN_DLD(_oct_project, args, nargout, "Oct project") {
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
        patch(r, c) = interp(image, image_pt(0), image_pt(1));  
      } else {
        patch(r, c) = ::octave_NA;
      }
    }
  }
  std::cout << ::octave_NA << std::endl;
  return octave_value(patch);
}

#if DEFTEST

#include <octave/octave.h>
#include <octave/toplev.h>

#include <vw/Camera/PinholeModel.h>
#include <vw/Image/UtilityViews.h>
#include <vw/Image/Filter.h>
#include <vw/Image/MaskViews.h>
#include <vw/Image/Statistics.h>
#include <vw/Image/ImageView.h>
#include <vw/FileIO.h>

#include <mvp/Octave/oct-cast.h>

#include <boost/random/linear_congruential.hpp>

using namespace std;
using namespace vw;
using namespace vw::camera;
using namespace mvp::octave;

octave_value_list synth_orbit(Vector3 const& camera_center, Vector3 const& axis_angle, 
                              Vector2 const& focus, Vector2i size)
{
  octave_value_list result;

  boost::rand48 gen;
  ImageView<PixelMask<double> > im = gaussian_filter(uniform_noise_view(gen, size.x(), size.y()), 1.5);
  result.append(octave_cast<octave_value>(im));

  PinholeModel camera(camera_center, axis_angle_to_matrix(axis_angle), 
                      focus.x(), focus.y(), size.x() / 2.0, size.y() / 2.0);
  result.append(octave_cast<octave_value>(camera.camera_matrix()));

  return result; 
}

template <class Image1T, class Image2T>
bool is_image_valid_eq(ImageViewBase<Image1T> const& img1, ImageViewBase<Image2T> const& img2, double tol) {
  double diff = double(sum_of_channel_values(apply_mask(abs(img1.impl() - img2.impl()))));
  int nvalid = sum_of_channel_values(apply_mask(copy_mask(constant_view<int>(1, img1.impl()), img1.impl() - img2.impl())));
  return nvalid && diff < tol; 
}

int main()  {
  const char * argvv [] = {"", "--silent"};
  ::octave_main (2, (char **) argvv, true);

  Vector2i size(64, 32);

  // Camera at z = 1, looking back at origin. Plane at z = 0
  octave_value_list trial = synth_orbit(Vector3(0, 0, 1), Vector3(M_PI, 0, 0), Vector2(1, 1), size);
  trial.append(octave_cast<octave_value>(Vector3(0, 0, 0)));
  trial.append(octave_cast<octave_value>(Quat(1, 0, 0, 0)));
  trial.append(octave_cast<octave_value>(Vector2(1, 1)));
  trial.append(octave_cast<octave_value>(size));

  ImageView<PixelMask<double> > result = octave_cast<ImageView<PixelMask<double> > >(F_oct_project(trial, 6)(0));
  ImageView<PixelMask<double> >  orig = octave_cast<ImageView<PixelMask<double> > >(trial(0));

  write_image("result.tif", channel_cast_rescale<uint8>(result));
  write_image("orig.tif", channel_cast_rescale<uint8>(orig));

  cout << is_image_valid_eq(result, orig, 1e-6) << endl;

  return 0;
}


#endif

/*

%!test
%! function c = orbital_image(center, orientation, focus, sz)
%!   intrinsic = eye(3);
%!   intrinsic(1, 1) = focus(1);
%!   intrinsic(2, 2) = focus(2);
%!   intrinsic(1, 3) = sz(1) / 2;
%!   intrinsic(2, 3) = sz(2) / 2;
%! endfunction
%!
%!
%!
%!

*/

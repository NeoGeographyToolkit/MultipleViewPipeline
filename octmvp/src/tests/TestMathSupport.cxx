#include <gtest/gtest.h>

#include "MathSupport.h"

TEST(bilinear_interp2, calculate) {
  Matrix im(2, 2);
  im(0, 0) = 1; im(0, 1) = 2;
  im(1, 0) = 3; im(1, 1) = 4;

  EXPECT_EQ(bilinear_interp2(im, 0, 0), 1); 
  EXPECT_EQ(bilinear_interp2(im, 1, 0), 2); 
  EXPECT_EQ(bilinear_interp2(im, 0, 1), 3); 
  EXPECT_EQ(bilinear_interp2(im, 1, 1), 4); 
  EXPECT_EQ(bilinear_interp2(im, 0.5, 0.5), (1 + 2 + 3 + 4) / 4.0);

  EXPECT_EQ(bilinear_interp2(im, 0.25, 0.25), 1.75);
}

TEST(quat2rot, calculate) {
  ColumnVector quat(4);
  quat(0) = 5;
  quat(1) = 6;
  quat(2) = 8;
  quat(3) = 10;
  quat /= 15;

  Matrix rot = quat2rot(quat);

  EXPECT_NEAR(rot(0, 0), -0.457778, 1e-4); EXPECT_NEAR(rot(0, 1), -0.017778, 1e-4); EXPECT_NEAR(rot(0, 2), 0.888889, 1e-4);
  EXPECT_NEAR(rot(1, 0), 0.871111, 1e-4); EXPECT_NEAR(rot(1, 1), -0.208889, 1e-4); EXPECT_NEAR(rot(1, 2), 0.444444, 1e-4);
  EXPECT_NEAR(rot(2, 0), 0.177778, 1e-4); EXPECT_NEAR(rot(2, 1), 0.977778, 1e-4); EXPECT_NEAR(rot(2, 2), 0.111111, 1e-4);
}

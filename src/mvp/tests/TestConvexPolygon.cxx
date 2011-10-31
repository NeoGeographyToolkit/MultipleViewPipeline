#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/ConvexPolygon.h>

using namespace std;
using namespace mvp;
using namespace vw;
using namespace vw::test;

TEST(PolygonMath, isec_poly) {
  ConvexPolygon::VertexList list1, list2, list3, list4;

  list1.push_back(Vector2(30, 40));
  list1.push_back(Vector2(30, 80));
  list1.push_back(Vector2(50, 80));
  list1.push_back(Vector2(50, 40));

  list2.push_back(Vector2(40, 30));
  list2.push_back(Vector2(40, 70));
  list2.push_back(Vector2(60, 70));
  list2.push_back(Vector2(60, 30));

  list3.push_back(Vector2(45, 50));
  list3.push_back(Vector2(45, 60));
  list3.push_back(Vector2(47, 60));
  list3.push_back(Vector2(47, 50));

  list4.push_back(Vector2(48, 20));
  list4.push_back(Vector2(48, 60));
  list4.push_back(Vector2(70, 60));
  list4.push_back(Vector2(70, 20));

  EXPECT_TRUE(ConvexPolygon(list1).intersects(ConvexPolygon(list2)));
  EXPECT_TRUE(ConvexPolygon(list1).intersects(ConvexPolygon(list3)));
  EXPECT_TRUE(ConvexPolygon(list2).intersects(ConvexPolygon(list3)));
  EXPECT_TRUE(ConvexPolygon(list1).intersects(ConvexPolygon(list4)));
  EXPECT_TRUE(ConvexPolygon(list2).intersects(ConvexPolygon(list4)));
  EXPECT_FALSE(ConvexPolygon(list3).intersects(ConvexPolygon(list4)));
}

TEST(PolygonMath, circulation_direction) {
  EXPECT_GT(circulation_direction(Vector2(2, 2), Vector2(3, 3), Vector2(1, 5)), 0);
  EXPECT_LT(circulation_direction(Vector2(1, 5), Vector2(3, 3), Vector2(2, 2)), 0);
  EXPECT_EQ(circulation_direction(Vector2(2, 2), Vector2(3, 3), Vector2(4, 4)), 0);
}

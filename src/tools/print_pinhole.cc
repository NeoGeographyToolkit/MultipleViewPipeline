

#include <vw/Camera.h>

using namespace std;
using namespace vw;
using namespace vw::camera;

void print_vector3(Vector3 vect)
{
  cout << "vw::Vector3(";
  cout << vect[0] << ",";
  cout << vect[1] << ",";
  cout << vect[2] << ")";
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    cout << "Usage: " << argv[0] << " camera_model.pinhole" << endl;
    return 1;
  }

  PinholeModel cam(argv[1]); 

  cout << setprecision(10);
  cout << "Camera center: " << cam.camera_center() << endl;
  cout << "Camera pose: " << cam.camera_pose() << endl;
  cout << "Focal length: " << cam.focal_length() << endl;
  cout << "Point offset: " << cam.point_offset() << endl;
  cout << "U dir: " << cam.coordinate_frame_u_direction() << endl;
  cout << "V dir: " << cam.coordinate_frame_v_direction() << endl;
  cout << "W dir: " << cam.coordinate_frame_w_direction() << endl;

  cout << "vw::camera::PinholeModel(";
  print_vector3(cam.camera_center());
  cout << "," << endl;
  cout << "  vw::Quat(";
  cout << cam.camera_pose()[0] << ",";
  cout << cam.camera_pose()[1] << ",";
  cout << cam.camera_pose()[2] << ",";
  cout << cam.camera_pose()[3] << ").rotation_matrix()," << endl;
  cout << "  ";
  cout << cam.focal_length()[0] << ",";
  cout << cam.focal_length()[1] << ",";
  cout << cam.point_offset()[0] << ",";
  cout << cam.point_offset()[1] << "," << endl;
  cout << "  ";
  print_vector3(cam.coordinate_frame_u_direction());
  cout << "," << endl << "  ";
  print_vector3(cam.coordinate_frame_v_direction());
  cout << "," << endl << "  ";
  print_vector3(cam.coordinate_frame_w_direction());
  cout << "," << endl << "  ";
  cout << "vw::camera::NullLensDistortion());" << endl;

  return 0;
}

#include <iostream>

#include <mvp/MVPJobRequest.pb.h>

int main(int argc, char* argv[])
{
  using namespace std;
  using namespace mvp;

  cout << "Hello world" << endl;

  MVPJobRequest job;
  cout << job.plate_georef().tile_size() << endl;

  return 0;
}

#include <iostream>

#include <MVPJobRequest.pb.h>

int main(int argc, char* argv[])
{
  using namespace std;
  using namespace mvp;

  cout << "Hello world" << endl;

  MVPJobRequest job;
  job.set_tile_size(6);
  cout << job.tile_size() << endl;

  return 0;
}
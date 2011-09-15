
class MVPAlgorithmImpl {
  public:

    virtual double operator()(x, y, ..) const = 0;
};

struct MVPAlgorithmOctave : MVPAlgorithmImpl {

  MVPAlgorithmOctave(MVPAlgorithmDesc desc) {
    // Package up desc into octave struct
  }

  virtual double operator()(x, y, ..) const {
    feval("do_mvp_algorithm", algoname, octavestruct)  

  }

};

class MVPAlgorithm {
  boost::shared_ptr<MVPAlgorithmImpl> impl;

  public:
    static const vector<string> algorithm_library;

    MVPAlgorithm(MVPAlgorithmDesc desc) {
      if (desc.use_octave) {
        impl = new MVPAlgorithmOctave(desc);        
      } else {
        FOR_EACH(algo, algorithm_library) {
          if (algo.name == desc.algoname) {
            impl = new algo(desc);
            break;
          }
        }
        if (!impl) {
          vw_assert(logic_err);
        }
      }
    }

    double operator()(int x, int y, GeoReference georef, ImageList, CameraList) {
      return (*impl)(x, y, georef, ImageList, CameraList);
    }
};

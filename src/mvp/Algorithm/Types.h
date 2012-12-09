/// \file Types.h
///
/// Defines types used in algorithms
///
/// TODO: Write something here
///

#include <vw/Math/Quaternion.h>
#include <vw/Image/ImageView.h>
#include <vw/Image/PixelMask.h>
#include <vw/FileIO/DiskImageResource.h>

#ifndef __MVP_ALGORITHM_TYPES_H__
#define __MVP_ALGORITHM_TYPES_H__

namespace mvp {
namespace algorithm {

class AlgorithmVar {
  public:
    enum {numel = 12};
    typedef vw::Vector<double, numel> raw_type;

    AlgorithmVar() : m_data() {}

    AlgorithmVar(raw_type data) : m_data(data) {}

    raw_type& data() {return m_data;}

    raw_type const& data() const {return m_data;}

    double alt() const {return m_data[0];}

    vw::Quat orientation() const {return vw::Quat(m_data[1], m_data[2], m_data[3], m_data[4]);}

    vw::Vector2 window() const {return vw::Vector2(m_data[5], m_data[6]);}

    vw::Vector2 gwindow() const {return vw::Vector2(m_data[7], m_data[8]);}

    double smooth() const {return m_data[9];}

    double gsmooth() const {return m_data[10];}

    double scale() const {return m_data[11];}

    void set_alt(double alt) {m_data[0] = alt;}

    void set_orientation(vw::Quat const& orientation) {
      m_data[1] = orientation[0];
      m_data[2] = orientation[1];
      m_data[3] = orientation[2];
      m_data[4] = orientation[3];
    }

    void set_window(vw::Vector2 const& window) {
      m_data[5] = window[0];
      m_data[6] = window[1];
    }

    void set_gwindow(vw::Vector2 const& gwindow) {
      m_data[7] = gwindow[0];
      m_data[8] = gwindow[1];
    }

    void set_smooth(double smooth) {m_data[9] = smooth;}

    void set_gsmooth(double gsmooth) {m_data[10] = gsmooth;}

    void set_scale(double scale) {m_data[11] = scale;}

  private:
    raw_type m_data;
};

class PixelResult {
  public:
    PixelResult() : m_algorithm_var(), m_data(), m_converged(false) {}

    PixelResult(AlgorithmVar const& var) : 
      m_algorithm_var(var), m_data(), m_converged(false) {}

    PixelResult(AlgorithmVar::raw_type const& var) : 
      m_algorithm_var(var), m_data(), m_converged(false) {}

    AlgorithmVar const& algorithm_var() const {return m_algorithm_var;}

    vw::PixelMask<vw::float32> plate_layer(int layer) const {
      vw::PixelMask<vw::float32> result;
      
      if (layer < AlgorithmVar::numel) {
        result = m_algorithm_var.data()[layer];  
      } else {
        result = m_data[layer - AlgorithmVar::numel];
      }

      if (!m_converged) {
        result.invalidate();
      }

      return result;
    }

    void set_plate_layer(int layer, vw::PixelMask<vw::float32> const& val) {
      if (layer < AlgorithmVar::numel) {
        m_algorithm_var.data()[layer] = val;
      } else {
        m_data[layer - AlgorithmVar::numel] = val;
      }
      m_converged = vw::is_valid(val);
    }

    int num_plate_layers() const {return numel + AlgorithmVar::numel;}

    double confidence() const {return m_data[0];}

    int overlap() const {return round(m_data[1]);}

    bool converged() const {return m_converged;}

    void set_confidence(double confidence) {m_data[0] = confidence;}

    void set_overlap(int overlap) {m_data[1] = overlap;}

    void set_converged(bool converged) {m_converged = converged;}

  private:
    enum {numel = 2};
    typedef vw::Vector<double, numel> raw_type;

    AlgorithmVar m_algorithm_var;
    raw_type m_data; 
    bool m_converged;
};

class TileResult {
  int m_tile_size;
  boost::shared_array<PixelResult> m_data;

  PixelResult& at_internal(int c, int r) {return m_data[c + m_tile_size*r];}
  PixelResult const& at_internal(int c, int r) const {return m_data[c + m_tile_size*r];}

  vw::ImageView<vw::PixelMask<vw::float32> > plate_layer(int layer) const {
    vw::ImageView<vw::PixelMask<vw::float32> > result(m_tile_size, m_tile_size);
    for (int r = 0; r < m_tile_size; r++) {
      for (int c = 0; c < m_tile_size; c++) {
        result(c, r) = at_internal(c, r).plate_layer(layer);
      }
    }
    return result;
  }

  void set_plate_layer(int layer, vw::ImageView<vw::PixelMask<vw::float32> > const& data) {
    for (int r = 0; r < m_tile_size; r++) {
      for (int c = 0; c < m_tile_size; c++) {
        at_internal(c, r).set_plate_layer(layer, data(c, r));
      }
    }
  }

  public:

    TileResult(std::string const& platefile, int tile_row, int tile_col, int tile_level, int tile_size) : m_tile_size(tile_size), m_data(new PixelResult[tile_size*tile_size]) {}

    PixelResult const& at(int c, int r) {return at_internal(c, r);}

    void set(int c, int r, PixelResult const& val) {at_internal(c, r) = val;}

    void write() {
      // Open plate, write all layers
    }

    void debug_write(std::string const& prefix) {
      // Write to individual files
      write_image("alt.tif", plate_layer(0));
    }

    void read() {
      // Read from platefile
    }
};

}}

#endif

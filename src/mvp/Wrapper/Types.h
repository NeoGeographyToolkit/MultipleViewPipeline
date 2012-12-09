/// \file Types.h
///
/// TODO: Write doc
///

#ifndef __MVP_WRAPPER_TYPES_H__
#define __MVP_WRAPPER_TYPES_H__

#include <mvp/Wrapper/MvpWrapper.h>

#include <mvp/Algorithm/Types.h>

MVP_WRAPPER_BEGIN(AlgorithmVar, mvp::algorithm::AlgorithmVar) {
  
  MVP_WRAPPER_CONSTRUCT1(mvp::algorithm::AlgorithmVar, "AlgorithmVar", mvp::algorithm::AlgorithmVar::raw_type);

  MVP_WRAPPER_FUNCTION(mvp::algorithm::AlgorithmVar::raw_type, data);

  MVP_WRAPPER_FUNCTION(double, alt);
  MVP_WRAPPER_FUNCTION(vw::Quat, orientation);
  MVP_WRAPPER_FUNCTION(vw::Vector2, window);
  MVP_WRAPPER_FUNCTION(vw::Vector2, gwindow);
  MVP_WRAPPER_FUNCTION(double, smooth);
  MVP_WRAPPER_FUNCTION(double, gsmooth);
  MVP_WRAPPER_FUNCTION(double, scale);

  MVP_WRAPPER_VOID1(set_alt, double);
  MVP_WRAPPER_VOID1(set_orientation, vw::Quat);
  MVP_WRAPPER_VOID1(set_window, vw::Vector2);
  MVP_WRAPPER_VOID1(set_gwindow, vw::Vector2);
  MVP_WRAPPER_VOID1(set_smooth, double);
  MVP_WRAPPER_VOID1(set_gsmooth, double);
  MVP_WRAPPER_VOID1(set_scale, double);

} MVP_WRAPPER_END

namespace mvp {
namespace octave {
/// mvp::AlgorithmVar -> octave
template <class ToT>
ToT octave_cast(mvp::algorithm::AlgorithmVar const& var) {
  return ToT(new octave_mvpobj_ref(new octave_mvpobj_wrap<mvp::algorithm::AlgorithmVar>(new mvp::algorithm::AlgorithmVar(var), true)));
}
}}

MVP_WRAPPER_BEGIN(PixelResult, mvp::algorithm::PixelResult) {
  
  MVP_WRAPPER_CONSTRUCT1(mvp::algorithm::PixelResult, "PixelResult", mvp::algorithm::AlgorithmVar::raw_type);

  MVP_WRAPPER_FUNCTION(mvp::algorithm::AlgorithmVar, algorithm_var);

  MVP_WRAPPER_FUNCTION(double, confidence);
  MVP_WRAPPER_FUNCTION(int, overlap);
  MVP_WRAPPER_FUNCTION(bool, converged);

  MVP_WRAPPER_VOID1(set_confidence, double);
  MVP_WRAPPER_VOID1(set_overlap, int);
  MVP_WRAPPER_VOID1(set_converged, bool);

} MVP_WRAPPER_END

MVP_WRAPPER_BEGIN(TileResult, mvp::algorithm::TileResult) {
  

} MVP_WRAPPER_END  

#endif

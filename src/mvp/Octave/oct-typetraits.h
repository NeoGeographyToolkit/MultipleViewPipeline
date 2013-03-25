/// \file oct-typetraits.h
///
/// Helpers with octave types
///

#ifndef __MVP_OCTAVE_OCTTYPETRAITS_H__
#define __MVP_OCTAVE_OCTTYPETRAITS_H__

#include <octave/ov.h>

template <class T>
struct octave_array_type {
  typedef intNDArray<octave_int<T> > type;
  typedef octave_int<T> value_type;
};

#define OCTAVE_ARRAY_TYPE(T, TYPE) \
template <> \
struct octave_array_type<T> { \
  typedef TYPE type; \
  typedef T value_type; \
};

OCTAVE_ARRAY_TYPE(bool, boolNDArray)
OCTAVE_ARRAY_TYPE(float, FloatNDArray)
OCTAVE_ARRAY_TYPE(double, NDArray)

#endif

/// \file OctaveWrapper.h
///
/// Wrap octave classes so they can be
/// accessed in C++
///

#ifndef BOOST_PP_IS_ITERATING

#ifndef __MVP_OCTAVE_OCTAVEWRAPPER_H__
#define __MVP_OCTAVE_OCTAVEWRAPPER_H__

#include <mvp/Octave/Conversions.h>

#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/control/expr_if.hpp>

#define OCTAVE_WRAPPER_MAX_ARITY 10

namespace mvp {
namespace octave {

class OctaveWrapperImpl {
  octave_value m_impl;

  static octave_value create_impl(std::string const& impl_name, octave_value_list const& args);

  octave_value wrap_function_impl(std::string const& func, octave_value_list const& args);

  public:
    OctaveWrapperImpl(octave_value const& impl) : m_impl(impl) {}

  #define BOOST_PP_ITERATION_LIMITS (0, OCTAVE_WRAPPER_MAX_ARITY)
  #define BOOST_PP_FILENAME_1 <mvp/Octave/OctaveWrapper.h>
  #include BOOST_PP_ITERATE()

};

}} // namespace octave, mvp

#define BEGIN_OCTAVE_WRAPPER(NAME, IMPLT) \
namespace { \
typedef IMPLT ImplT; \
struct OctaveWrapper : public ImplT { \
  mutable mvp::octave::OctaveWrapperImpl m_octave_impl; \
  OctaveWrapper(octave_value const& impl) : m_octave_impl(impl) {}

#define OCTAVE_WRAP_args_helper(r, x, n, t) BOOST_PP_COMMA_IF(n) t BOOST_PP_CAT(a, n)

#define OCTAVE_WRAP_CONSTRUCTOR(ARGS) \
  OctaveWrapper(BOOST_PP_SEQ_FOR_EACH_I(OCTAVE_WRAP_args_helper, ~, ARGS)) : \
    m_octave_impl(BOOST_PP_ENUM_PARAMS(BOOST_PP_SEQ_SIZE(ARGS), a)) {}

#define OCTAVE_WRAP_helper(FUNC, RET, ARGS, CONST) \
  RET FUNC(BOOST_PP_SEQ_FOR_EACH_I(OCTAVE_WRAP_args_helper, ~, ARGS)) BOOST_PP_EXPR_IF(CONST, const) { \
    using namespace mvp::octave; \
    return from_octave<RET>(m_octave_impl.wrap_function(BOOST_PP_STRINGIZE(FUNC) \
                                                        BOOST_PP_ENUM_TRAILING_PARAMS(BOOST_PP_SEQ_SIZE(ARGS), a) )); \
  }

#define OCTAVE_WRAP(FUNC, SIG) OCTAVE_WRAP_helper(FUNC, BOOST_PP_SEQ_HEAD(SIG), BOOST_PP_SEQ_TAIL(SIG), 0)
#define OCTAVE_WRAP_C(FUNC, SIG) OCTAVE_WRAP_helper(FUNC, BOOST_PP_SEQ_HEAD(SIG), BOOST_PP_SEQ_TAIL(SIG), 1)

#define END_OCTAVE_WRAPPER() \
}; \
} \
template <> \
ImplT octave_wrapper<ImplT>(octave_value const& impl) { \
  return OctaveWrapper(impl); \
} \
REGISTER_OCTAVE_ALGORITHM_OBJECTS(OctaveWrapper)

#endif

#else // BOOST_PP_IS_ITERATING

#define N BOOST_PP_ITERATION()
#define OCTAVE_WRAPPER_arg_helper(z, n, data) args.append(to_octave(BOOST_PP_CAT(a, n)))

#if N
template <BOOST_PP_ENUM_PARAMS(N, class T)>
OctaveWrapperImpl(BOOST_PP_ENUM_BINARY_PARAMS(N, T, a)) {
  std::string impl_name = BOOST_PP_CAT(a, BOOST_PP_SUB(N, 1)).impl_name();
  octave_value_list args;
  BOOST_PP_ENUM(N, OCTAVE_WRAPPER_arg_helper, ~);
  m_impl = create_impl(impl_name, args);
}
#endif

#if N
template <BOOST_PP_ENUM_PARAMS(N, class T)>
#endif
octave_value wrap_function(std::string const& func BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(N, T, a)) {
  octave_value_list args;
  BOOST_PP_ENUM(N, OCTAVE_WRAPPER_arg_helper, ~);
  return wrap_function_impl(func, args);
}

#undef N
#undef OCTAVE_WRAP_args_helper

#endif // BOOST_PP_IS_ITERATING

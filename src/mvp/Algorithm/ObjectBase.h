#ifndef BOOST_PP_IS_ITERATING

#ifndef __MVP_ALGORITHM_OBJECTBASE_H__
#define __MVP_ALGORITHM_OBJECTBASE_H__

#include <map>

#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>

#include <vw/Core/Exception.h>

namespace mvp {
namespace algorithm {


#define MVP_ALGORITHM_MAX_ARITY 10

template <class ImplT, BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(MVP_ALGORITHM_MAX_ARITY, class T, void)>
class ObjectBase;

#define BOOST_PP_ITERATION_LIMITS (1, MVP_ALGORITHM_MAX_ARITY)
#define BOOST_PP_FILENAME_1 <mvp/Algorithm/ObjectBase.h>
#include BOOST_PP_ITERATE()

template <class DerivedT>
struct ObjectRegistrar {
  static ObjectRegistrar<DerivedT> reg;
  ObjectRegistrar(std::string const& name) {
    DerivedT::template register_object<DerivedT>(name);
  } 
};

#define REGISTER_ALGORITHM_OBJECT(Derived) \
  template <> ObjectRegistrar<Derived> ObjectRegistrar<Derived>::reg(#Derived);

template <class FactoryT>
struct OctaveObjectRegistrar {
  static OctaveObjectRegistrar<FactoryT> reg;
  // The dummy argument in the constructor is needed
  // otherwise the static variable is not initialized at program start
  OctaveObjectRegistrar(int hack) {
    FactoryT::template register_octave_factory<FactoryT>();
  } 
};

#define REGISTER_OCTAVE_ALGORITHM_OBJECTS(Factory) \
  template <> mvp::algorithm::OctaveObjectRegistrar<Factory> mvp::algorithm::OctaveObjectRegistrar<Factory>::reg(1);

}} // namespace mvp, algorithm

#endif

#else // BOOST_PP_IS_ITERATING

#define N BOOST_PP_ITERATION()

template <class ImplT BOOST_PP_ENUM_TRAILING_PARAMS(N, class T)>
class ObjectBase
#if N != MVP_ALGORITHM_MAX_ARITY
<ImplT BOOST_PP_ENUM_TRAILING_PARAMS(N, T)>
#endif
{
  static std::map<std::string, boost::function<ImplT*(BOOST_PP_ENUM_PARAMS(N, T))> >& factory_map() {
    static std::map<std::string, boost::function<ImplT*(BOOST_PP_ENUM_PARAMS(N, T))> > map;
    return map; 
  }

  static boost::function<ImplT*(BOOST_PP_ENUM_PARAMS(N, T))>& octave_factory() {
    static boost::function<ImplT*(BOOST_PP_ENUM_PARAMS(N, T))> factory;
    return factory;
  }

  boost::shared_ptr<ImplT> m_impl;

  protected:
    ObjectBase() {}

    ObjectBase(BOOST_PP_ENUM_BINARY_PARAMS(N, T, a)) {
      std::string impl_name = BOOST_PP_CAT(a, BOOST_PP_SUB(N, 1)).impl_name();
      bool use_octave = BOOST_PP_CAT(a, BOOST_PP_SUB(N, 1)).use_octave();
      if (use_octave) {
        VW_ASSERT(octave_factory(), vw::LogicErr() << "No octave impl factory registered");
        m_impl.reset(octave_factory()(BOOST_PP_ENUM_PARAMS(N, a)));
      } else {
        VW_ASSERT(factory_map().count(impl_name), vw::ArgumentErr() << impl_name << " not found in constructor map");
        m_impl.reset(factory_map()[impl_name](BOOST_PP_ENUM_PARAMS(N, a)));
      }        
    }

    boost::shared_ptr<ImplT> impl() { 
      VW_ASSERT(m_impl, vw::LogicErr() << "Method not defined in derived object class");
      return m_impl;
    }

    boost::shared_ptr<ImplT> impl() const { 
      VW_ASSERT(m_impl, vw::LogicErr() << "Method not defined in derived object class");
      return m_impl;
    }

  public:
    template <class DerivedT>
    static void register_object(std::string const& name) {
      factory_map()[name] = boost::factory<DerivedT*>();
    }

    template <class FactoryT>
    static void register_octave_factory() {
      octave_factory() = boost::factory<FactoryT*>();
    }
};

#undef N

#endif // BOOST_PP_IS_ITERATING

#ifndef BOOST_PP_IS_ITERATING

#ifndef __MVP_ALGORITHM_ALGOBASE_H__
#define __MVP_ALGORITHM_ALGOBASE_H__

#include <map>

#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>

#include <vw/Core/Exception.h>

namespace mvp {
namespace algorithm {

template <class ConstructT> struct AlgoBase;

#define MVP_ALGORITHM_MAX_ARITY 10

#define BOOST_PP_ITERATION_LIMITS (0, MVP_ALGORITHM_MAX_ARITY)
#define BOOST_PP_FILENAME_1 <mvp/Algorithm/AlgoBase.h>
#include BOOST_PP_ITERATE()

template <class DerivedT>
struct AlgoRegistrar {
  static AlgoRegistrar<DerivedT> reg;
  AlgoRegistrar(std::string const& name) {
    DerivedT::template register_algorithm<DerivedT>(name);
  } 
};

#define REGISTER_ALGORITHM(Derived) \
  template <> AlgoRegistrar<Derived> AlgoRegistrar<Derived>::reg(#Derived);

template <class FactoryT>
struct OctaveAlgoRegistrar {
  static OctaveAlgoRegistrar<FactoryT> reg;
  // The dummy argument in the constructor is needed
  // otherwise the static variable is not initialized at program start
  OctaveAlgoRegistrar(int hack) {
    FactoryT::template register_octave_factory<FactoryT>();
  } 
};

#define REGISTER_OCTAVE_ALGORITHMS(Factory) \
  template <> mvp::algorithm::OctaveAlgoRegistrar<Factory> mvp::algorithm::OctaveAlgoRegistrar<Factory>::reg(1);

}} // namespace mvp, algorithm

#endif

#else // BOOST_PP_IS_ITERATING

#define N BOOST_PP_ITERATION()

template <class AlgoT BOOST_PP_ENUM_TRAILING_PARAMS(N, class T)>
struct AlgoBase<AlgoT(BOOST_PP_ENUM_PARAMS(N, T))> {
  typedef AlgoT algorithm_type;

  private:

    static std::map<std::string, boost::function<AlgoT*(BOOST_PP_ENUM_PARAMS(N, T))> >& factory_map() {
      static std::map<std::string, boost::function<AlgoT*(BOOST_PP_ENUM_PARAMS(N, T))> > map;
      return map; 
    }

    static boost::function<AlgoT*(std::string const& BOOST_PP_ENUM_TRAILING_PARAMS(N, T))>& octave_factory() {
      static boost::function<AlgoT*(std::string const& BOOST_PP_ENUM_TRAILING_PARAMS(N, T))> factory;
      return factory;
    }

    boost::shared_ptr<AlgoT> m_impl;

  protected:
    AlgoBase() {}

    AlgoBase(std::string const& algo_name, bool use_octave BOOST_PP_ENUM_TRAILING_BINARY_PARAMS(N, T, a)) {
      if (use_octave) {
        VW_ASSERT(octave_factory(), vw::LogicErr() << "No octave impl factory registered");
        m_impl.reset(octave_factory()(algo_name BOOST_PP_ENUM_TRAILING_PARAMS(N, a)));
      } else {
        VW_ASSERT(factory_map().count(algo_name), vw::ArgumentErr() << algo_name << " not found in constructor map");
        m_impl.reset(factory_map()[algo_name](BOOST_PP_ENUM_PARAMS(N, a)));
      }        
    }

    boost::shared_ptr<AlgoT> impl() { 
      VW_ASSERT(m_impl, vw::LogicErr() << "Method not defined in derived algorithm class");
      return m_impl;
    }

    boost::shared_ptr<AlgoT> impl() const { 
      VW_ASSERT(m_impl, vw::LogicErr() << "Method not defined in derived algorithm class");
      return m_impl;
    }

  public:
    template <class DerivedT>
    static void register_algorithm(std::string const& name) {
      factory_map()[name] = boost::factory<DerivedT*>();
    }

    template <class FactoryT>
    static void register_octave_factory() {
      octave_factory() = boost::factory<FactoryT*>();
    }
};

#undef N

#endif // BOOST_PP_IS_ITERATING

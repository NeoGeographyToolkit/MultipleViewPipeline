#ifndef __MVP_ALGORITHM_ALGOBASE_H__
#define __MVP_ALGORITHM_ALGOBASE_H__

#include <map>

#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <vw/Core/Exception.h>

namespace mvp {
namespace algorithm {

template <class ConstructT> struct AlgoBase;

template <class AlgoT, class T0, class T1>
struct AlgoBase<AlgoT(T0, T1)> {
  typedef AlgoT algorithm_type;

  private:

    static std::map<std::string, boost::function<AlgoT*(T0, T1)> >& factory_map() {
      static std::map<std::string, boost::function<AlgoT*(T0, T1)> > map;
      return map; 
    }

    static boost::function<AlgoT*(std::string const&, T0, T1)>& octave_factory() {
      static boost::function<AlgoT*(std::string const&, T0, T1)> factory;
      return factory;
    }

    boost::shared_ptr<AlgoT> m_impl;

  protected:
    AlgoBase() {}

    AlgoBase(std::string const& algo_name, bool use_octave, T0 a0, T1 a1) {
      if (use_octave) {
        VW_ASSERT(octave_factory(), vw::LogicErr() << "No octave impl factory registered");
        m_impl.reset(octave_factory()(algo_name, a0, a1));
      } else {
        VW_ASSERT(factory_map().count(algo_name), vw::ArgumentErr() << "Type not found in constructor map");
        m_impl.reset(factory_map()[algo_name](a0, a1));
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
  template <> OctaveAlgoRegistrar<Factory> OctaveAlgoRegistrar<Factory>::reg(1);

}} // namespace mvp, algorithm

#endif

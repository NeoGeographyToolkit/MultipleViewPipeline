#ifndef __MVP_ALGORITHM_ALGOBASE_H__
#define __MVP_ALGORITHM_ALGOBASE_H__

#include <map>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>

#include <vw/Core/Exception.h>
#include <vw/Core/Log.h>

namespace mvp {
namespace algorithm {

template <class ConstructT>
struct AlgoBase {
  typedef typename boost::remove_pointer<typename boost::function_traits<typename boost::remove_pointer<ConstructT>::type>::result_type>::type algorithm_type;

  private:

  static std::map<std::string, boost::function<ConstructT> >& factory_map() {
    static std::map<std::string, boost::function<ConstructT> > map;
    return map; 
  }

  boost::shared_ptr<algorithm_type> m_impl;


  protected:
    AlgoBase() {}

    AlgoBase(algorithm_type *impl) : m_impl(impl) {}

    boost::shared_ptr<algorithm_type> impl() { 
      if (!m_impl) {
        vw::vw_throw(vw::LogicErr() << "Method not defined in derived algorithm class");
      }
      return m_impl;
    }

    static boost::function<ConstructT> lookup_constructor(std::string const& type) {
      if (!factory_map().count(type)) {
        vw::vw_throw(vw::LogicErr() << "Type not found in constructor map");
      } 
      return factory_map()[type];
    }

  public:
    template <class DerivedT>
    static void register_algorithm(std::string const& name) {
      vw::vw_out(vw::DebugMessage, "mvpalgorithm") << "Registered algorithm: " << name << std::endl;
      factory_map()[name] = boost::factory<DerivedT*>();
    }
};

template <class BaseT, class DerivedT>
struct AlgoRegistrar {
  AlgoRegistrar(std::string const& name) {
    BaseT::template register_algorithm<DerivedT>(name);
  } 
};

#define REGISTER_ALGORITHM(Base, Derived) \
  static AlgoRegistrar<Base, Derived> s_reg;

#define REGISTER_ALGORITHM_IMPL(Base, Derived)\
  AlgoRegistrar<Base, Derived> Derived::s_reg(#Derived);

}} // namespace mvp, algorithm

#endif

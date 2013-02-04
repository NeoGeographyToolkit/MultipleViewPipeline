#ifndef __MVP_ALGORITHM_ALGOBASE_H__
#define __MVP_ALGORITHM_ALGOBASE_H__

#include <map>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>

#include <vw/Core/Exception.h>

namespace mvp {
namespace algorithm {

template <class ConstructT>
class AlgoBase {
  typedef typename boost::remove_pointer<typename boost::function_traits<typename boost::remove_pointer<ConstructT>::type>::result_type>::type AlgoT;

  static std::map<std::string, boost::function<ConstructT> > s_factory;
  static bool s_factory_inited;
  boost::shared_ptr<AlgoT> m_impl;

  protected:
    AlgoBase() {}

    AlgoBase(AlgoT *impl) : m_impl(impl) {}

    boost::shared_ptr<AlgoT> impl() { 
      if (!m_impl) {
        vw::vw_throw(vw::LogicErr() << "Method not defined in derived algorithm class");
      }
      return m_impl;
    }

    static boost::function<ConstructT> lookup_constructor(std::string const& type) {
      if (!s_factory_inited) {
        AlgoT::register_all_algorithms();
        s_factory_inited = true;
      } 

      if (!s_factory.count(type)) {
        vw::vw_throw(vw::LogicErr() << "Type not found in constructor map");
      } 
      return s_factory[type];
    }

  public:
    template <class DerrivedT>
    static void register_algorithm(std::string const& name) {
      s_factory[name] = boost::factory<DerrivedT*>();
    }
};

template <class ConstructT>
std::map<std::string, boost::function<ConstructT> > AlgoBase<ConstructT>::s_factory;

template <class ConstructT>
bool AlgoBase<ConstructT>::s_factory_inited = false;

}} // namespace mvp, algorithm

#endif

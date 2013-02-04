#ifndef __MVP_ALGORITHM_ALGOBASE_H__
#define __MVP_ALGORITHM_ALGOBASE_H__

#include <map>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/shared_ptr.hpp>

#include <vw/Core/Exception.h>

namespace mvp {
namespace algorithm {

template <class AlgoT>
class AlgoBase {
  static std::map<std::string, boost::function<AlgoT*()> > s_factory;
  static bool s_factory_inited;
  boost::shared_ptr<AlgoT> m_impl;

  protected:
    boost::shared_ptr<AlgoT> impl() { 
      if (!m_impl) {
        vw::vw_throw(vw::LogicErr() << "Method not defined in derived algorithm class");
      }

      return m_impl;
    }
    
    void set_impl(std::string const& type) {
      if (s_factory.count(type)) {
        m_impl.reset(s_factory[type]());
      } else{  
        vw::vw_throw(vw::LogicErr() << "Type not found in constructor map");
      }
    }

  public:
    template <class DerrivedT>
    static void register_algorithm(std::string const& name) {
      s_factory[name] = boost::factory<DerrivedT*>();
    }
};

template <class AlgoT>
std::map<std::string, boost::function<AlgoT*()> > AlgoBase<AlgoT>::s_factory;

template <class AlgoT>
bool AlgoBase<AlgoT>::s_factory_inited = false;

}} // namespace mvp, algorithm

#endif

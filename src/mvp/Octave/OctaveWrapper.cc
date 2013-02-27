#include <octave/config.h>

#include <mvp/Octave/OctaveWrapper.h>

#include <octave/symtab.h>
#include <octave/parse.h>

#include <vw/Core/Exception.h>

namespace mvp {
namespace octave {

octave_value OctaveWrapperImpl::create_impl(std::string const& impl_name, octave_value_list const& args) {
  octave_value cnstr = symbol_table::find_function(impl_name);

  VW_ASSERT(cnstr.is_defined(), vw::NotFoundErr() << "Unable to find octave class " << impl_name);

  octave_value_list res = feval(cnstr.function_value(), args);

  VW_ASSERT(res.length() == 1, vw::LogicErr() << "Unable to construct octave class " << impl_name);

  return res(0);
}

octave_value OctaveWrapperImpl::wrap_function_impl(std::string const& func, octave_value_list const& args) {
  std::list<octave_value_list> idx;
  idx.push_back(octave_value(func));
  idx.push_back(args);

  return m_impl.subsref(".(", idx);
} 

}} // namespace octave, mvp

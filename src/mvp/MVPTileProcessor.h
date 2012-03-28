/// \file MVPTileProcessor.h
///
/// MVP Tile Processor
///
/// TODO: Write something here
///

#ifndef __MVP_MVPTILEPROCESSOR_H__
#define __MVP_MVPTILEPROCESSOR_H__

#include <mvp/MVPTypes.h>

#include <vw/Core/ProgressCallback.h>

namespace mvp {

class MVPTileSeeder;

struct MVPTileProcessor {
  virtual MVPTileResult operator()(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const = 0;
};

class MVPTileProcessorDumb : public MVPTileProcessor {
  MVPTileSeeder *m_seeder;

  public:
    MVPTileProcessorDumb(MVPTileSeeder *seeder) : m_seeder(seeder) {}
    virtual MVPTileResult operator()(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const;  
};

} // namespace mvp

#endif

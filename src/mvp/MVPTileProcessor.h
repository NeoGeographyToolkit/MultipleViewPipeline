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

class MVPTileProcessor {
  protected:
    MVPTileSeeder *m_seeder;
  public:
    MVPTileProcessor(MVPTileSeeder *seeder) : m_seeder(seeder) {}
    virtual MVPTileResult operator()(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const = 0;
};

struct MVPTileProcessorDumb : public MVPTileProcessor {
  MVPTileProcessorDumb(MVPTileSeeder *seeder) : MVPTileProcessor(seeder) {}
  virtual MVPTileResult operator()(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const;  
};

} // namespace mvp

#endif

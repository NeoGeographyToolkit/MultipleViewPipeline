mvp_module(NAME Algorithm
           DEPS mvpCore
                mvpImage
           HDRS Dummy.h
           SRCS Dummy.cc
                Dummy/DerivedDummy.cc
                AlgorithmVar.cc
                PixelResult.cc
                Seed.cc
                TileResult.cc
                PatchBox.cc
                AlbedoBox.cc
                Lighter.cc
                Objective.cc
                Stepper.cc
                Seeder.cc
                Correlator.cc
           PROTOS DummySettings.proto
                  LighterSettings.proto
                  ObjectiveSettings.proto
                  StepperSettings.proto
                  SeederSettings.proto
                  CorrelatorSettings.proto
           )

#if (BUILD_TESTS)
#  add_subdirectory(tests)
#endif()

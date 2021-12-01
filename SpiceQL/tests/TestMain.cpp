#include <gtest/gtest.h>
#include "Fixtures.h"

int main(int argc, char **argv) {
   testing::Environment* const spiceql_env = testing::AddGlobalTestEnvironment(new TempTestingFiles);

   ::testing::InitGoogleTest(&argc, argv);
   return RUN_ALL_TESTS();
}
#include <gtest/gtest.h>
#include <fmt/format.h>

#include "utils.h"
#include "Fixtures.h"
#include "spice_types.h"
#include "query.h"

#include "SpiceUsr.h"

using namespace SpiceQL;


TEST_F(LroKernelSet, UnitTestStackedKernelConstructorDestructor) {
  int nkernels;

  // This should create local kernels that get unfurnished when the stack is popped
  {
    Kernel k(lskPath);

    // should match what spice counts
    ktotal_c("text", &nkernels);

    // base LSK still loaded
    EXPECT_EQ(nkernels, 2);
    EXPECT_EQ(pool.getRefCounts().at(lskPath), 1);
  }

  // SCLKs and LSKs are considered text kernels, so they should stay loaded
  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 1);
  EXPECT_EQ(pool.getRefCount(lskPath), 0);
}


TEST_F(LroKernelSet, UnitTestStackedKernelCopyConstructor) {
  int nkernels;

  // This should create local kernels that get unfurnished when the stack is popped
  {
    Kernel k(lskPath);
    Kernel k2 = k;
    Kernel k3(k2);

    // should match what spice counts
    ktotal_c("text", &nkernels);

    // 5 total text kernels, but the lsk should have been loaded 3 times
    EXPECT_EQ(nkernels, 4);
    EXPECT_EQ(pool.getRefCounts().at(lskPath), 3);
  }

  // SCLKs and LSKs are considered text kernels, so they should stay loaded
  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 1);
  EXPECT_EQ(pool.getRefCount(lskPath), 0);
}


TEST_F(LroKernelSet, UnitTestStackedKernelSetConstructorDestructor) {
  // load all available kernels
  nlohmann::json kernels = searchMissionKernels(root, conf);

  // do a time query
  kernels = searchMissionKernels(kernels, {110000000, 120000001}, false);

  // get only latest versions
  kernels = getLatestKernels(kernels);

  // all the kernels in the group are now furnished.
  KernelSet ks(kernels);

  int nkernels;

  // load kernels in a closed call stack
  {
    // kernels are now loaded twice
    KernelSet k(kernels);

    // should match what spice counts
    ktotal_c("text", &nkernels);
    EXPECT_EQ(nkernels, 7);
    ktotal_c("ck", &nkernels);
    EXPECT_EQ(nkernels, 2);
    ktotal_c("spk", &nkernels);
    EXPECT_EQ(nkernels, 2);

    // 5 because LSK is not being loaded (yet)
    EXPECT_EQ(pool.getRefCounts().size(), 6);
    EXPECT_EQ(pool.getRefCount(fkPath), 2);
    EXPECT_EQ(pool.getRefCount(ckPath1), 2);
    EXPECT_EQ(pool.getRefCount(spkPath1), 2);
    EXPECT_EQ(pool.getRefCount(sclkPath), 2);
    EXPECT_EQ(pool.getRefCount(ikPath2), 2);
  }

  // All kernels in previous stack should be unfurnished
  ktotal_c("text", &nkernels);
  EXPECT_EQ(nkernels, 4);
  ktotal_c("ck", &nkernels);
  EXPECT_EQ(nkernels, 1);
  ktotal_c("spk", &nkernels);
  EXPECT_EQ(nkernels, 1);

  EXPECT_EQ(pool.getRefCounts().size(), 6);
  EXPECT_EQ(pool.getRefCount(fkPath), 1);
  EXPECT_EQ(pool.getRefCount(ckPath1), 1);
  EXPECT_EQ(pool.getRefCount(spkPath1), 1);
  EXPECT_EQ(pool.getRefCount(sclkPath), 1);
  EXPECT_EQ(pool.getRefCount(ikPath2), 1);
}


TEST_F(LroKernelSet, UnitTestStackedKernelPoolGetLoadedKernels) {
  // load all available kernels
  nlohmann::json kernels = searchMissionKernels(root, conf);

  // do a time query
  kernels = searchMissionKernels(kernels, {110000000, 120000001}, false);
  kernels = getLatestKernels(kernels);

  KernelSet k(kernels);

  std::vector<string> kv = pool.getLoadedKernels();
  EXPECT_EQ(kv.size(), 6);
  EXPECT_TRUE(std::find(kv.begin(), kv.end(), fkPath) != kv.end());
  EXPECT_TRUE(std::find(kv.begin(), kv.end(), ckPath1) != kv.end());
  EXPECT_TRUE(std::find(kv.begin(), kv.end(), spkPath1) != kv.end());
  EXPECT_TRUE(std::find(kv.begin(), kv.end(), sclkPath) != kv.end());
  EXPECT_TRUE(std::find(kv.begin(), kv.end(), ikPath2) != kv.end());
}


TEST_F(LroKernelSet, UnitTestLoadTimeKernels) {
  vector<string> kv = pool.getLoadedKernels();
  set<string> expected = {"naif0011.tls", "lro_clkcor_2020184_v00.tsc"};

  for (auto & e: kv) {
    EXPECT_TRUE(expected.find(static_cast<fs::path>(e).filename()) != expected.end());
  }
}
#include "Fixtures.h"
#include "Paths.h"

#include <exception>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>


#include "utils.h"
#include "io.h"

using namespace std;
using namespace SpiceQL;

void TempTestingFiles::SetUp() {
  int max_tries = 10;
  auto tmp_dir = fs::temp_directory_path();
  unsigned long long i = 0;
  random_device dev;
  mt19937 prng(dev());
  uniform_int_distribution<uint64_t> rand(0);
  fs::path tpath;

  while (true) {
    stringstream ss;
    ss << "SSTESTS" << hex << rand(prng);
    tpath = tmp_dir / ss.str();

    // true if the directory was created.
    if (fs::create_directory(tpath)) {
        break;
    }
    if (i == max_tries) {
        throw runtime_error("could not find non-existing directory");
    }
    i++;
  }
  tempDir = tpath;
}


void TempTestingFiles::TearDown() {
    if(!fs::remove_all(tempDir)) {
      throw runtime_error("Could not delete temporary files");
    }
}


void KernelDataDirectories::SetUp() {
  // combine multiple path lists here as we add more.
  paths = base_paths;
  paths.insert(paths.end(), mess_paths.begin(), mess_paths.end());
  paths.insert(paths.end(), clem1_paths.begin(), clem1_paths.end());
  paths.insert(paths.end(), galileo_paths.begin(), galileo_paths.end());
  paths.insert(paths.end(), lro_paths.begin(), lro_paths.end());
}


void KernelDataDirectories::TearDown() {

}

void KernelSet::SetUp() {
  TempTestingFiles::SetUp();
  root = tempDir;

  // Move Clock kernels
  // TODO: Programmatic clock kernels
  fs::path lskPath = fs::path("data") / "naif0012.tls";
  fs::path sclkPath = fs::path("data") / "lro_clkcor_2020184_v00.tsc";
  create_directory(tempDir / "clocks");

  fs::copy_file(lskPath, tempDir / "clocks" / "naif0012.tls");
  fs::copy_file(sclkPath, tempDir / "clocks" / "lro_clkcor_2020184_v00.tsc");

  // Write CK1 ------------------------------------------
  fs::create_directory(tempDir / "ck");

  int bodyCode = -85000;
  std::string referenceFrame = "j2000";

  fs::path ckPath1 = tempDir / "ck" / "soc31.0001.bc";
  std::vector<std::vector<double>> avs = {{1,1,1}, {2,2,2}};
  std::vector<std::vector<double>> quats = {{0.2886751, 0.2886751, 0.5773503, 0.7071068 }, {0.4082483, 0.4082483, 0.8164966, 0 }};
  std::vector<double> times1 = {110000000, 120000000};
  std::vector<double> times2 = {130000000, 140000000};

  writeCk(ckPath1, quats, times1, bodyCode, referenceFrame, "CK ID 1",  sclkPath, lskPath, avs, "CK1");

  // Write CK2 ------------------------------------------
  fs::path ckPath2 = tempDir / "ck" / "lrolc.0002.bc";
  avs = {{3,4,5}, {6,5,5}};
  quats = {{0.3754439, 0.3754439, 0.3754439, -0.7596879}, {-0.5632779, -0.5632779, -0.5632779, 0.21944}};
  writeCk(ckPath2, quats, times2, bodyCode, referenceFrame, "CK ID 2", sclkPath, lskPath, avs, "CK2");

  // Write SPK1 ------------------------------------------
  fs::create_directory(tempDir / "spk");
  fs::path spkPath1 = tempDir / "spk" / "LRO_TEST_GRGM660MAT270.bsp";

  std::vector<std::vector<double>> velocities = {{1,1,1}, {2,2,2}};
  std::vector<std::vector<double>> positions = {{1, 1, 1}, {2, 2, 2}};
  writeSpk(spkPath1, positions, times1, bodyCode, 1, referenceFrame, "SPK ID 1", 1, velocities, "SPK 1");

  // Write SPK2 ------------------------------------------
  velocities = {{3, 3, 3}, {5, 5, 5}};
  positions = {{3, 3, 3}, {4, 4, 4}};
  fs::path spkPath2 = tempDir / "spk" / "LRO_TEST_GRGM660MAT370.bsp";
  writeSpk(spkPath2, positions, times2, bodyCode, 1, referenceFrame, "SPK ID 2", 1, velocities, "SPK 2");

  // Write IK1 -------------------------------------------
  fs::create_directory(tempDir / "ik");

  fs::path ikPath1 = tempDir / "ik" / "lro_instruments_v10.ti";
  nlohmann::json jKeywords = {
    {"INS-85600_PIXEL_SAMPLES", { 5064 }},
    {"INS-85600_PIXEL_LINES", { 1 }},
    {"INS-85600_PIXEL_SIZE", { 7.0E-3 , 7.0E-3 }},
    {"INS-85600_CCD_CENTER", { 2531.5 , 0.5 }}
  };

  writeTextKernel(ikPath1, "ik", jKeywords);

  // Write IK2 -------------------------------------------
  fs::path ikPath2 = tempDir / "ik" / "lro_instruments_v11.ti";
  jKeywords = {
    {"INS-85600_PIXEL_SAMPLES", { 5063 }},
    {"INS-85600_PIXEL_LINES", { 1 }},
    {"INS-85600_PIXEL_SIZE", { 7.0E-4 , 7.0E-2 }},
    {"INS-85600_CCD_CENTER", { 2531.3 , 0.4 }}
  };

  writeTextKernel(ikPath2, "ik", jKeywords);

  // Write FK ---------------------------------------------
  fs::create_directory(tempDir / "fk");

  jKeywords = {
    {"FRAME_LRO_LROCWAC", -85620},
    {"FRAME_-85620_NAME", "LRO_LROCWAC"},
    {"FRAME_-85620_CLASS", 3},
    {"FRAME_-85620_CLASS_ID", -85620},
    {"FRAME_-85620_CENTER", -85},
    {"TKFRAME_-85620_RELATIVE", "LRO_SC_BUS"},
    {"CK_-85620_SCLK", -85},
    {"CK_-85620_SPK", -85}
  };

  fs::path fkPath = tempDir / "fk" / "lro_frames_1111111_v01.tf";

  writeTextKernel(fkPath, "fk", jKeywords);
}

void KernelSet::TearDown() {

}
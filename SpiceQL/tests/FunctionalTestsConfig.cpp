#include <fstream>

#include <gtest/gtest.h>

#include "Fixtures.h"

#include "config.h"
#include "utils.h"

using namespace std;
using json = nlohmann::json;
using namespace SpiceQL;

TEST_F(TestConfig, FunctionalTestConfigConstruct) {
  json megaConfig = testConfig.getRawConfig();

  ASSERT_EQ(megaConfig.size(), 25);
}

TEST_F(TestConfig, FunctionalTestConfigEval) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);
  mocks.OnCallFunc(getDataDirectory).Return("/isis_data/");

  json config_eval_res = testConfig.getJson();
  json pointer_eval_res = testConfig.getJson("/clem1", true);

  json::json_pointer pointer = "/clem1/ck/reconstructed/kernels"_json_pointer;
  int expected_number = 4;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/clem1/ck/smithed/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/clem1/spk/reconstructed/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/clem1/fk/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/clem1/sclk/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(pointer_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);


  pointer = "/uvvis/ik/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);

  pointer = "/uvvis/iak/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
  ASSERT_EQ(testConfig[pointer].size(), expected_number);
}


TEST_F(TestConfig, FunctionalTestConfigGlobalEval) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);
  mocks.OnCallFunc(getDataDirectory).Return("/isis_data/");

  testConfig.getJson();
  json config_eval_res = testConfig.getRawConfig();

  json::json_pointer pointer = "/clem1/ck/reconstructed/kernels"_json_pointer;
  int expected_number = 4;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clem1/ck/smithed/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clem1/spk/reconstructed/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clem1/fk/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/clem1/sclk/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/uvvis/ik/kernels"_json_pointer;
  expected_number = 1;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);

  pointer = "/uvvis/iak/kernels"_json_pointer;
  expected_number = 2;
  ASSERT_EQ(config_eval_res[pointer].size(), expected_number);
}

TEST_F(TestConfig, FunctionalTestConfigAccessors) {
  Config base = testConfig["base"];
  Config base_pointer = testConfig["/base"];
  
  MockRepository mocks; 
  mocks.OnCallFunc(SpiceQL::ls).Return(paths);

  EXPECT_EQ(base.getJson()["lsk"]["kernels"].at(0), "/isis_data/base/kernels/sclk/naif0001.tls");
  EXPECT_EQ(base_pointer.getJson()["lsk"]["kernels"].at(0), "/isis_data/base/kernels/sclk/naif0001.tls");
}

TEST_F(TestConfig, FunctionalTestsConfigKeySearch) {
  vector<json::json_pointer> pointers = {"/my/first/pointer"_json_pointer, "/my/second/pointer"_json_pointer};

  MockRepository mocks;
  mocks.OnCallFunc(SpiceQL::findKeyInJson).Return(pointers);

  vector<string> res_pointers = testConfig.findKey("kernels", true);

  int i = 0;
  for (auto pointer : res_pointers) {
    EXPECT_EQ(pointer, pointers[i++]);
  }
}

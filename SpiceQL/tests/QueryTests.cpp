#include <fstream>
#include <algorithm>

#include <HippoMocks/hippomocks.h>

#include <gtest/gtest.h>

#include "Fixtures.h"

#include "query.h"
#include "utils.h"


using namespace std;
using namespace SpiceQL;


TEST(QueryTests, UnitTestGetLatestKernel) {
  vector<string> kernels = {
    "iak.0001.ti",
    "iak.0003.ti",
    "different/place/iak.0002.ti",
    "test/iak.0004.ti"
  };

  EXPECT_EQ(getLatestKernel(kernels),  "test/iak.0004.ti");
}

TEST(QueryTests, getKernelStringValue){
  unique_ptr<Kernel> k(new Kernel("data/msgr_mdis_v010.ti"));
  // INS-236810_CCD_CENTER        =  (  511.5, 511.5 )
  EXPECT_EQ(getKernelStringValue("INS-236810_FOV_SHAPE"), "RECTANGLE");

  try {
    getKernelStringValue("aKeyThatWillNotBeInTheResults");
    FAIL() << "Expected std::invalid_argument";
  }
  catch(std::invalid_argument const & err) {
      EXPECT_EQ(err.what(),std::string("key not in results"));
  }
  catch(...) {
      FAIL() << "Expected std::invalid_argument";
  }
}


TEST(QueryTests, getKernelVectorValue){
  unique_ptr<Kernel> k(new Kernel("data/msgr_mdis_v010.ti"));

  vector<string> actualResultsOne = getKernelVectorValue("INS-236810_CCD_CENTER");
  std::vector<string> expectedResultsOne{"511.5", "511.5"};

  vector<string> actualResultsTwo = getKernelVectorValue("INS-236800_FOV_REF_VECTOR");
  std::vector<string> expectedResultsTwo{"1.0", "0.0", "0.0"};

  for (int i = 0; i < actualResultsOne.size(); ++i) {
    EXPECT_EQ(actualResultsOne[i], expectedResultsOne[i]) << "Vectors x and y differ at index " << i;
  }

  for (int j = 0; j < actualResultsTwo.size(); ++j) {
    EXPECT_EQ(actualResultsTwo[j], expectedResultsTwo[j]) << "Vectors x and y differ at index " << j;
  }

   try {
        getKernelVectorValue("aKeyThatWillNotBeInTheResults");
        FAIL() << "Expected std::invalid_argument";
    }
    catch(std::invalid_argument const & err) {
        EXPECT_EQ(err.what(),std::string("key not in results"));
    }
    catch(...) {
        FAIL() << "Expected std::invalid_argument";
    }
}

TEST(QueryTests, UnitTestGetLatestKernelError) {
  vector<string> kernels = {
    "iak.0001.ti",
    "iak.0003.ti",
    "different/place/iak.0002.ti",
    "test/iak.4.ti",
    // different extension means different filetype and therefore error
    "test/error.tf"
  };

  try {
    getLatestKernel(kernels);
    FAIL() << "expected invalid argument error";
  }
  catch(invalid_argument &e) {
    SUCCEED();
  }
}


TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsAllMess) {
  string dbPath = getMissionConfigFile("mess");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  ASSERT_EQ(res["mdis"]["ck"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["mdis"]["ck"]["smithed"]["kernels"].size(), 4);
  ASSERT_EQ(res["mdis"]["ck"]["deps"]["objs"].size(), 4);
  ASSERT_EQ(res["mdis"]["spk"]["reconstructed"]["kernels"].size(), 2);
  ASSERT_EQ(res["mdis"]["tspk"]["kernels"].size(), 1);
  ASSERT_EQ(res["mdis"]["fk"]["kernels"].size(), 2);
  ASSERT_EQ(res["mdis"]["ik"]["kernels"].size(), 2);
  ASSERT_EQ(res["mdis"]["iak"]["kernels"].size(), 2);
  ASSERT_EQ(res["mdis"]["pck"]["na"]["kernels"].size(), 2);

  ASSERT_EQ(res["mdis_att"]["ck"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["mdis_att"]["ck"]["deps"]["objs"].size(), 2);

  ASSERT_EQ(res["mess"]["ck"]["reconstructed"]["kernels"].size(), 5);
  ASSERT_EQ(res["mess"]["sclk"]["kernels"].size(), 2);
  ASSERT_EQ(res["mess"]["ck"]["deps"]["objs"].size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsClem1) {
  fs::path dbPath = getMissionConfigFile("clem1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  ASSERT_EQ(res["clem1"]["ck"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["clem1"]["ck"]["smithed"]["kernels"].size(), 1);
  ASSERT_EQ(res["clem1"]["ck"]["deps"]["objs"].size(), 2);
  ASSERT_EQ(res["clem1"]["spk"]["reconstructed"]["kernels"].size(), 2);
  ASSERT_EQ(res["clem1"]["fk"]["kernels"].size(), 1);
  ASSERT_EQ(res["clem1"]["sclk"]["kernels"].size(), 2);

  ASSERT_EQ(res["uvvis"]["ik"]["kernels"].size(), 1);
  ASSERT_EQ(res["uvvis"]["iak"]["kernels"].size(), 2);

  ASSERT_EQ(res["uvvis"]["iak"]["kernels"].size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsGalileo) {
  fs::path dbPath = getMissionConfigFile("galileo");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  ASSERT_EQ(res["galileo"]["ck"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["galileo"]["ck"]["reconstructed"]["deps"].size(), 0);
  ASSERT_EQ(res["galileo"]["ck"]["smithed"]["kernels"].size(), 3);
  ASSERT_EQ(res["galileo"]["ck"]["smithed"]["deps"]["objs"].size(), 1);
  ASSERT_EQ(res["galileo"]["ck"]["deps"]["objs"].size(), 2);
  ASSERT_EQ(res["galileo"]["spk"]["reconstructed"]["kernels"].size(), 2);
  ASSERT_EQ(res["galileo"]["iak"]["kernels"].size(), 1);
  ASSERT_EQ(res["galileo"]["pck"]["smithed"]["kernels"].size(), 2);
  ASSERT_EQ(res["galileo"]["pck"]["smithed"]["deps"].size(), 0);
  ASSERT_EQ(res["galileo"]["pck"]["na"]["kernels"].size(), 1);
  ASSERT_EQ(res["galileo"]["pck"]["na"]["deps"].size(), 0);
  ASSERT_EQ(res["galileo"]["sclk"]["kernels"].size(), 1);
}



TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsCassini) {
  fs::path dbPath = getMissionConfigFile("cassini");
  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  ASSERT_EQ(res["cassini"]["ck"]["reconstructed"]["kernels"].size(), 2);
  ASSERT_EQ(res["cassini"]["ck"]["smithed"]["kernels"].size(), 3);



  ASSERT_EQ(res["cassini"]["fk"]["kernels"].size(), 2);

  ASSERT_EQ(res["cassini"]["iak"]["kernels"].size(), 3);

  
  ASSERT_EQ(res["cassini"]["pck"]["deps"].size(), 1);
  ASSERT_EQ(res["cassini"]["pck"]["kernels"].size(), 3);
  ASSERT_EQ(res["cassini"]["pck"]["smithed"]["kernels"].size(), 1);
  ASSERT_EQ(res["cassini"]["sclk"]["kernels"].size(), 1);
  ASSERT_EQ(res["cassini"]["spk"]["kernels"].size(), 3);
}

TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsApollo16) {
  fs::path dbPath = getMissionConfigFile("apollo16");
    ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;
  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  ASSERT_EQ(res["apollo16"]["sclk"]["kernels"].size(), 1);
  ASSERT_EQ(res["apollo16"]["ck"]["reconstructed"]["kernels"].size(), 4);

  ASSERT_EQ(res["apollo16"]["spk"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["apollo16"]["fk"]["kernels"].size(), 2);
  ASSERT_EQ(res["metric"]["ik"]["kernels"].size(), 2);
  ASSERT_EQ(res["metric"]["iak"]["kernels"].size(), 1);
  ASSERT_EQ(res["panoramic"]["ik"]["kernels"].size(), 1);
  ASSERT_EQ(res["apollo_pan"]["iak"]["kernels"].size(), 2);
}

// test for apollo 17 kernels 
TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsApollo17) {
  fs::path dbPath = getMissionConfigFile("apollo17");


  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);


  ASSERT_EQ(res["apollo17"]["ck"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["apollo17"]["sclk"]["kernels"].size(), 1);
  ASSERT_EQ(res["apollo17"]["fk"]["kernels"].size(), 2);
  ASSERT_EQ(res["apollo17"]["spk"]["reconstructed"]["kernels"].size(), 4);
  ASSERT_EQ(res["METRIC"]["iak"]["kernels"].size(), 1);
  ASSERT_EQ(res["PANORAMIC"]["ik"]["kernels"].size(), 3);
  ASSERT_EQ(res["APOLLO_PAN"]["iak"]["kernels"].size(), 2);
}

TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsLRO) {
  fs::path dbPath = getMissionConfigFile("lro");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  EXPECT_EQ(res["lroc"]["ck"]["reconstructed"]["kernels"].size(), 8);
  EXPECT_EQ(res["lroc"]["ck"]["deps"]["objs"].size(), 3);
  EXPECT_EQ(res["lroc"]["spk"]["reconstructed"]["kernels"].size(), 8);
  EXPECT_EQ(res["lroc"]["spk"]["smithed"]["kernels"].size(), 14);
  EXPECT_EQ(res["lroc"]["iak"]["kernels"].size(), 2);
  EXPECT_EQ(res["lroc"]["ik"]["kernels"].size(), 2);
  EXPECT_EQ(res["lroc"]["pck"]["kernels"].size(), 2);
  EXPECT_EQ(res["lroc"]["fk"]["kernels"].size(), 2);
  EXPECT_EQ(res["lroc"]["tspk"]["kernels"].size(), 2);
}


TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsJuno) {
  fs::path dbPath = getMissionConfigFile("jno");

  string base = "/isis_data/juno";
  regex base_reg(fmt::format("({})", fmt::join(base, "")));
  vector<string> paths_with_base;

  for (auto path: paths) {
    if (regex_search(path, base_reg)) {
      paths_with_base.push_back(path);
    }
  }

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths_with_base);

  nlohmann::json res = searchMissionKernels(base, conf);

  ASSERT_EQ(res["juno"]["ck"]["reconstructed"]["kernels"].size(), 4); 
  ASSERT_EQ(res["juno"]["ck"]["deps"]["objs"].size(), 2); 
  ASSERT_EQ(res["juno"]["spk"]["reconstructed"]["kernels"].size(), 2); 
  ASSERT_EQ(res["juno"]["spk"]["deps"]["objs"].size(), 1); 
  ASSERT_EQ(res["juno"]["sclk"]["kernels"].size(), 1);  
  ASSERT_EQ(res["juno"]["tspk"]["kernels"].size(), 3);  
  ASSERT_EQ(res["juno"]["fk"]["kernels"].size(), 1); 
  ASSERT_EQ(res["juno"]["ik"]["kernels"].size(), 1);  
  ASSERT_EQ(res["juno"]["iak"]["kernels"].size(), 1);  
  ASSERT_EQ(res["juno"]["pck"]["na"]["kernels"].size(), 1); 
} 


TEST_F(IsisDataDirectory, MroConfTest) {
  fs::path dbPath = getMissionConfigFile("mro");
  
  ifstream i(dbPath);
  nlohmann::json conf = nlohmann::json::parse(i);

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(files);

  nlohmann::json res = searchMissionKernels("doesn't matter", conf);

  set<string> kernels = getKernelSet(res);
  set<string> mission = missionMap.at("mro");
  
  EXPECT_EQ(kernels, mission); 
  
  // check a kernel from each regex exists in there quality groups
  vector<string> kernelToCheck =  jsonArrayToVector(res.at("mro").at("spk").at("reconstructed").at("kernels"));
  vector<string> expected = {"mro_cruise.bsp", "mro_ab.bsp", "mro_psp_rec.bsp", 
                             "mro_psp1.bsp", "mro_psp10.bsp", "mro_psp_rec.bsp", 
                             "mro_psp1_ssd_mro95a.bsp", "mro_psp27_ssd_mro110c.bsp"};
  
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }

  kernelToCheck = getKernelList(res.at("mro").at("spk").at("predicted")); 
  expected = {"mro_psp.bsp"};
  EXPECT_EQ(kernelToCheck, expected);

  kernelToCheck = getKernelList(res.at("mro").at("ck").at("reconstructed"));
  expected = {"mro_sc_psp_160719_160725.bc", "mro_sc_cru_060301_060310.bc", 
              "mro_sc_ab_060801_060831.bc", "mro_sc_psp_150324_150330_v2.bc"};
  for (auto &e : expected) { 
    auto it = find(kernelToCheck.begin(), kernelToCheck.end(), e);
    EXPECT_TRUE(it != kernelToCheck.end());
  }
}

TEST_F(KernelDataDirectories, FunctionalTestSearchMissionKernelsViking1) {
  fs::path dbPath = getMissionConfigFile("viking1");

  ifstream i(dbPath);
  nlohmann::json conf;
  i >> conf;

  MockRepository mocks;
  mocks.OnCallFunc(ls).Return(paths);

  nlohmann::json res = searchMissionKernels("/isis_data/", conf);

  cout << res <<endl;
  EXPECT_EQ(res["viking1"]["ck"]["reconstructed"]["kernels"].size(), 1);
  EXPECT_EQ(res["viking1"]["ck"]["smithed"]["kernels"].size(), 1);
  EXPECT_EQ(res["viking1"]["fk"]["kernels"].size(), 1);
  EXPECT_EQ(res["viking1"]["iak"]["kernels"].size(), 1);
  EXPECT_EQ(res["viking1"]["sclk"]["kernels"].size(), 2);
  EXPECT_EQ(res["viking1"]["spk"]["reconstructed"]["kernels"].size(), 2);
}
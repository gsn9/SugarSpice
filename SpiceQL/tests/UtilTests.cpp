#include <gtest/gtest.h>

#include "utils.h"
#include "Fixtures.h"
#include "spice_types.h"
#include "SpiceUsr.h"

using namespace SpiceQL;

TEST(UtilTests, GetKernelType) { 
}


TEST(UtilTests, GetFrameCode) { 
}


TEST(UtilTests, GetFrameName) {  
}


TEST(UtilTests, findKeywords) {
  Kernel k("data/msgr_mdis_v010.ti");

  nlohmann::json res = findKeywords("*");
  EXPECT_EQ(res.at("INS-236810_FOV_SHAPE"), "RECTANGLE");
  EXPECT_EQ(res.at("INS-236800_WAVELENGTH_RANGE")[1], 1040);
  EXPECT_EQ(res.at("INS-236800_IFOV"), 179.6);
}


TEST(UtilTests, findKeyInJson) {
  nlohmann::ordered_json j = R"(
    {
      "me" : "test",
      "l1a" : {
        "l2a" : 1,
        "me" : 2, 
        "l2b" : {
          "l3a" : "yay", 
          "me" : 1
        }
      }
    })"_json;

  std::vector<nlohmann::json::json_pointer> res = findKeyInJson(j, "me", true);

  EXPECT_EQ(res.at(0).to_string(), "/l1a/l2b/me");
  EXPECT_EQ(res.at(1).to_string(), "/l1a/me");
  EXPECT_EQ(res.at(2).to_string(), "/me");
}


TEST(UtilTests, resolveConfigDependencies) {
  nlohmann::json baseConfig = R"(
  {
    "mission_1" : {
      "mission_key" : {
        "array_key" : ["array_1", "array_2"]
      },
      "deps" : ["/mission_2/bad_key"]
    },
    "mission_2" : {
      "nested_key" : {
        "value_key" : "value_1"
      },
      "bad_key" : {
        "bad_nested_key" : "bad_value"
      }
    },
    "instrument" : {
      "instrument_key" : {
        "dummy_key" : "dummy_1"
      },
      "nested_key" : {
        "value_key" : "value_2",
        "deps" : ["/mission_2/nested_key"]
      },
      "deps" : ["/mission_1"]
    }
  })"_json;

  nlohmann::json resolvedConfig = baseConfig["instrument"];
  resolveConfigDependencies(resolvedConfig, baseConfig);

  nlohmann::json expectedConfig = R"(
  {
    "instrument_key" : {
      "dummy_key" : "dummy_1"
    },
    "mission_key" : {
      "array_key" : ["array_1", "array_2"]
    },
    "bad_nested_key" : "bad_value",
    "nested_key" : {
      "value_key" : ["value_2", "value_1"]
    }
  })"_json;
  EXPECT_EQ(resolvedConfig, expectedConfig);
}


TEST(UtilTests, resolveConfigDependenciesLoop) {
  nlohmann::json baseConfig = R"(
  {
    "Thing_1" : {
      "deps" : ["/Thing_2"]
    },
    "Thing_2" : {
      "deps" : ["/Thing_1"]
    }
  })"_json;

  nlohmann::json resolvedConfig = baseConfig["Thing_1"];
  EXPECT_THROW(resolveConfigDependencies(resolvedConfig, baseConfig), invalid_argument);
}


TEST(UtilTests, mergeConfigs) {
  nlohmann::json baseConfig = R"({
    "ck" : {
      "predict" : {
        "kernels" : "predict_ck_1.bc"
      },
      "reconstructed" : {
        "kernels" : "recon_ck_3.bs"
      },
      "smithed" : {
        "kernels" : ["smithed_ck_2.bs", "smithed_ck_3.bs"]
      }
    },
    "ik" : {
      "kernels" : "ik_1.ti"
    }
  })"_json;

  nlohmann::json mergeConfig = R"({
    "ck" : {
      "reconstructed" : {
        "kernels" : ["recon_ck_1.bc", "recon_ck_2.bc"]
      },
      "smithed" : {
        "kernels" : "smithed_ck_1.bs"
      }
    },
    "spk" : {
      "predict" : {
        "kernels" : "predict_spk_1.bsp"
      }
    },
    "sclk" : {
      "kernels" : "sclk_1.tsc"
    },
    "fk" : {
      "kernels" : "fk_1.tsc"
    }
  })"_json;

  mergeConfigs(baseConfig, mergeConfig);

  nlohmann::json expectedConfig = R"({
    "ck" : {
      "predict" : {
        "kernels" : "predict_ck_1.bc"
      },
      "reconstructed" : {
        "kernels" : ["recon_ck_3.bs", "recon_ck_1.bc", "recon_ck_2.bc"]
      },
      "smithed" : {
        "kernels" : ["smithed_ck_2.bs", "smithed_ck_3.bs", "smithed_ck_1.bs"]
      }
    },
    "spk" : {
      "predict" : {
        "kernels" : "predict_spk_1.bsp"
      }
    },
    "sclk" : {
      "kernels" : "sclk_1.tsc"
    },
    "fk" : {
      "kernels" : "fk_1.tsc"
    },
    "ik" : {
      "kernels" : "ik_1.ti"
    }
  })"_json;

  EXPECT_EQ(baseConfig, expectedConfig);

  nlohmann::json objectConfig = R"({
    "testkey" : {}
  })"_json;

  nlohmann::json valueConfig = R"({
    "testkey" : "testvalue"
  })"_json;

  EXPECT_THROW(mergeConfigs(objectConfig, valueConfig), std::invalid_argument);
  EXPECT_THROW(mergeConfigs(valueConfig, objectConfig), std::invalid_argument);
}

TEST(UtilTests, eraseAtPointer) {
  nlohmann::json baseJ = R"(
  {
    "outer_key" : {
      "middle_key" : {
        "inner_key" : {
          "value_to_erase" : "dummy"
        },
        "extra_inner_key" : "dummy"
      },
      "extra_middle_key" : "dummy"
    },
    "extra_outer_key" : "dummy"
  })"_json;

  nlohmann::json j = baseJ;
  size_t numErased = eraseAtPointer(j, nlohmann::json::json_pointer("/outer_key/middle_key/inner_key/value_to_erase"));
  EXPECT_EQ(numErased, 1);
  nlohmann::json expected = R"(
  {
    "outer_key" : {
      "middle_key" : {
        "inner_key" : {
        },
        "extra_inner_key" : "dummy"
      },
      "extra_middle_key" : "dummy"
    },
    "extra_outer_key" : "dummy"
  })"_json;
  EXPECT_EQ(j, expected);

  j = baseJ;
  numErased = eraseAtPointer(j, nlohmann::json::json_pointer("/outer_key/middle_key/bad_key"));
  EXPECT_EQ(numErased, 0);
  EXPECT_EQ(j, baseJ);
}
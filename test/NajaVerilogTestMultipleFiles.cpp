// SPDX-FileCopyrightText: 2023 The Naja verilog authors <https://github.com/keplertech/naja-verilog/blob/main/AUTHORS>
//
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>

#include "VerilogConstructor.h"

using namespace naja::verilog;

#include "VerilogConstructorTest.h"

#ifndef NAJA_VERILOG_BENCHMARKS
#define NAJA_VERILOG_BENCHMARKS "Undefined"
#endif

TEST(NajaVerilogTestMultipleFiles, test) {
  VerilogConstructorTest constructor;
  std::filesystem::path multipleFilesPath(
      std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
      / std::filesystem::path("multifiles"));
  VerilogConstructor::Paths paths;
  paths.push_back(multipleFilesPath / std::filesystem::path("mod0.v"));
  paths.push_back(multipleFilesPath / std::filesystem::path("mod1.v"));
  paths.push_back(multipleFilesPath / std::filesystem::path("top.v"));
  constructor.parse(paths);
  ASSERT_EQ(3, constructor.modules_.size());
  EXPECT_EQ("mod0", constructor.modules_[0]->identifier_.name_);
  EXPECT_EQ("mod1", constructor.modules_[1]->identifier_.name_);
  EXPECT_EQ("top", constructor.modules_[2]->identifier_.name_);
  ASSERT_EQ(2, constructor.modules_[0]->ports_.size());
  ASSERT_EQ(2, constructor.modules_[1]->ports_.size());
  ASSERT_EQ(2, constructor.modules_[2]->ports_.size());

  constructor.setFirstPass(false);
  constructor.parse(paths);

  EXPECT_TRUE(constructor.modules_[0]->instances_.empty());
  EXPECT_TRUE(constructor.modules_[1]->instances_.empty());
  ASSERT_EQ(2, constructor.modules_[2]->instances_.size());
  auto ins0 = dynamic_cast<VerilogConstructorTest::ModuleInstance*>(constructor.modules_[2]->instances_[0]);
  ASSERT_NE(nullptr, ins0);
  EXPECT_EQ("ins0", ins0->identifier_.name_);
  EXPECT_EQ("mod0", ins0->model_.name_);
  auto ins1 = dynamic_cast<VerilogConstructorTest::ModuleInstance*>(constructor.modules_[2]->instances_[1]);
  ASSERT_NE(nullptr, ins1);
  EXPECT_EQ("ins1", ins1->identifier_.name_);
  EXPECT_EQ("mod1", ins1->model_.name_);
}
TEST(NajaVerilogSourceRange, grammarSpans) {
  class Locations: public naja::verilog::VerilogConstructor {
    public:
      std::vector<SourceRange> ranges;
      void startModule(const naja::verilog::Identifier&) override { ranges.push_back(getCurrentSourceRange()); }
      void moduleInterfaceCompletePort(const naja::verilog::Port&) override { ranges.push_back(getCurrentSourceRange()); }
      void addNet(const naja::verilog::Net&) override { ranges.push_back(getCurrentSourceRange()); }
      void addInstance(const naja::verilog::Identifier&) override { ranges.push_back(getCurrentSourceRange()); }
  } constructor;
  auto path = std::filesystem::temp_directory_path() / "naja_verilog_source_ranges.v";
  {
    std::ofstream output(path);
    output << "module leaf(input a); endmodule\nmodule top();\n  wire x, y;\n  leaf u(x);\nendmodule\n";
  }
  constructor.parse(path);
  ASSERT_EQ(6, constructor.ranges.size());
  const auto& module = constructor.ranges[0];
  EXPECT_EQ(1, module.line);
  EXPECT_EQ(1, module.column);
  EXPECT_EQ(12, module.endColumn);
  const auto& port = constructor.ranges[1];
  EXPECT_EQ(13, port.column);
  EXPECT_EQ(20, port.endColumn);
  for (auto index: {3, 4}) {
    EXPECT_EQ(3, constructor.ranges[index].line);
    EXPECT_EQ(3, constructor.ranges[index].column);
    EXPECT_EQ(13, constructor.ranges[index].endColumn);
  }
  EXPECT_EQ(4, constructor.ranges[5].line);
  EXPECT_EQ(8, constructor.ranges[5].column);
  EXPECT_EQ(9, constructor.ranges[5].endColumn);
  {
    std::ofstream output(path);
    output << "module wide();\n" << std::string(70000, ' ') << "leaf long_name();\nendmodule\n";
  }
  constructor.ranges.clear();
  constructor.parse(path);
  ASSERT_EQ(2, constructor.ranges.size());
  EXPECT_EQ(70006, constructor.ranges[1].column);
  EXPECT_EQ(70015, constructor.ranges[1].endColumn);
  std::filesystem::remove(path);
}

// SPDX-FileCopyrightText: 2026 The Naja verilog authors
//
// SPDX-License-Identifier: Apache-2.0

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>

#include "VerilogConstructor.h"
#include "VerilogException.h"
#include "VerilogConstructorTest.h"
#include "VerilogPreprocessor.h"

#ifndef NAJA_VERILOG_BENCHMARKS
#define NAJA_VERILOG_BENCHMARKS "Undefined"
#endif

TEST(NajaVerilogTestPreprocess, includeDefineIfdef) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_top.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(2, constructor.modules_.size());
  EXPECT_EQ("inc_mod", constructor.modules_[0]->identifier_.name_);
  EXPECT_EQ("top", constructor.modules_[1]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, undefActiveAndInactive) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_undef.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("undef_mod", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, elsifElseBranches) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_elsif.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(2, constructor.modules_.size());
  EXPECT_EQ("elsif_mod", constructor.modules_[0]->identifier_.name_);
  EXPECT_EQ("else_taken", constructor.modules_[1]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, unclosedIfdefThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_unclosed_ifdef.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, defineInactiveBranchIgnored) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_define_inactive.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("define_inactive_mod", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, defineMissingNameThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_define_missing_name.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, undefMissingNameThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_undef_missing_name.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, ifdefMissingNameThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_ifdef_missing_name.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, ifndefMissingNameThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_ifndef_missing_name.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, elsifMissingNameThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_elsif_missing_name.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, elsifUnexpectedThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_unexpected_elsif.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, elsifReturnsInParentInactiveAndAnyTaken) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_elsif_returns.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("first_branch", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, parsePathsMissingFileThrows) {
  std::filesystem::path missingPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("does_not_exist.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  VerilogConstructorTest::Paths paths { missingPath };
  EXPECT_THROW(constructor.parse(paths), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, parsePathsUnreadableFileThrows) {
  auto tempPath = std::filesystem::temp_directory_path()
    / std::filesystem::path("naja_verilog_unreadable.v");
  {
    std::ofstream out(tempPath);
    out << "module unreadable(); endmodule\n";
  }
  std::filesystem::permissions(
    tempPath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write
      | std::filesystem::perms::owner_exec,
    std::filesystem::perm_options::remove);
  {
    std::ifstream probe(tempPath);
    if (probe.good()) {
      // Some environments (e.g., Emscripten test runners) do not enforce POSIX permissions.
      GTEST_SKIP() << "Filesystem does not honor unreadable permission in this environment.";
    }
  }

  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  VerilogConstructorTest::Paths paths { tempPath };
  EXPECT_THROW(constructor.parse(paths), naja::verilog::VerilogException);

  std::filesystem::permissions(
    tempPath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write,
    std::filesystem::perm_options::add);
  std::filesystem::remove(tempPath);
}

TEST(NajaVerilogTestPreprocess, elseUnexpectedThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_unexpected_else.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, elseParentInactiveReturns) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_else_parent_inactive.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("else_parent_inactive_ok", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, endifUnexpectedThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_unexpected_endif.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, includeInactiveReturns) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_include_inactive.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("include_inactive_mod", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, includeMissingQuotesThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_include_missing_quotes.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, includeMissingFileThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_include_missing_file.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, includeRecursiveThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_include_recursive.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, includeUnreadableThrows) {
  std::filesystem::path includePath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/include_unreadable.v"));
  {
    std::ofstream out(includePath);
    out << "module unreadable(); endmodule\n";
  }
  std::filesystem::permissions(
    includePath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write
      | std::filesystem::perms::owner_exec,
    std::filesystem::perm_options::remove);
  {
    std::ifstream probe(includePath);
    if (probe.good()) {
      // Some environments (e.g., Emscripten test runners) do not enforce POSIX permissions.
      GTEST_SKIP() << "Filesystem does not honor unreadable permission in this environment.";
    }
  }

  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_include_unreadable.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);

  std::filesystem::permissions(
    includePath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write,
    std::filesystem::perm_options::add);
  std::filesystem::remove(includePath);
}

TEST(NajaVerilogTestPreprocess, timescaleActiveKeepsLine) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_timescale_active.v"));
  naja::verilog::VerilogPreprocessor preprocessor;
  auto output = preprocessor.preprocessFile(testPath);
  EXPECT_EQ(std::string::npos, output.find("`timescale 1ns/1ps"));
}

TEST(NajaVerilogTestPreprocess, timescaleInactiveIgnored) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_timescale_inactive.v"));
  naja::verilog::VerilogPreprocessor preprocessor;
  auto output = preprocessor.preprocessFile(testPath);
  EXPECT_EQ(std::string::npos, output.find("`timescale 10ns/1ns"));
}

TEST(NajaVerilogTestPreprocess, unsupportedDirectiveThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_unsupported_directive.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, stringLiteralEscapesAreTokenized) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_string_escape.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.parse(testPath);
  constructor.setFirstPass(false);
  constructor.parse(testPath);
  ASSERT_EQ(1, constructor.modules_.size());
  const auto& assigns = constructor.modules_[0]->assigns_;
  ASSERT_EQ(3, assigns.size());
  EXPECT_EQ(naja::verilog::Expression::Type::STRING, assigns.back().expression_.value_.index());
  EXPECT_NE(std::string::npos, assigns.back().expression_.getString().find("`ESCAPED_MACRO"));
  EXPECT_NE(std::string::npos, assigns.back().expression_.getString().find('"'));
}

TEST(NajaVerilogTestPreprocess, preprocessToString) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_top.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  auto output = constructor.preprocessToString(testPath);
  EXPECT_NE(std::string::npos, output.find("module top"));
}

TEST(NajaVerilogTestPreprocess, preprocessToPath) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_top.v"));
  auto outputPath = std::filesystem::temp_directory_path()
    / std::filesystem::path("naja_verilog_preprocess.out.v");
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.preprocessToPath(testPath, outputPath);

  std::ifstream inFile(outputPath);
  ASSERT_TRUE(inFile.good());
  std::string contents((std::istreambuf_iterator<char>(inFile)),
                       std::istreambuf_iterator<char>());
  EXPECT_NE(std::string::npos, contents.find("module top"));

  std::filesystem::remove(outputPath);
}

TEST(NajaVerilogTestPreprocess, disablePreprocessBypassesMacroExpansion) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_top.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  constructor.setPreprocessEnabled(false);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, preprocessToStringMissingFileThrows) {
  std::filesystem::path missingPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/does_not_exist.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.preprocessToString(missingPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, preprocessToStringUnreadableFileThrows) {
  auto tempPath = std::filesystem::temp_directory_path()
    / std::filesystem::path("naja_verilog_preprocess_unreadable.v");
  {
    std::ofstream out(tempPath);
    out << "module unreadable(); endmodule\n";
  }
  std::filesystem::permissions(
    tempPath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write
      | std::filesystem::perms::owner_exec,
    std::filesystem::perm_options::remove);
  {
    std::ifstream probe(tempPath);
    if (probe.good()) {
      // Some environments (e.g., Emscripten test runners) do not enforce POSIX permissions.
      GTEST_SKIP() << "Filesystem does not honor unreadable permission in this environment.";
    }
  }
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.preprocessToString(tempPath), naja::verilog::VerilogException);

  std::filesystem::permissions(
    tempPath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write,
    std::filesystem::perm_options::add);
  std::filesystem::remove(tempPath);
}

TEST(NajaVerilogTestPreprocess, preprocessToPathUnwritableOutputThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_top.v"));
  auto outputPath = std::filesystem::temp_directory_path()
    / std::filesystem::path("naja_verilog_preprocess_nowrite.v");
  {
    std::ofstream out(outputPath);
    out << "placeholder\n";
  }
  std::filesystem::permissions(
    outputPath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write
      | std::filesystem::perms::owner_exec,
    std::filesystem::perm_options::remove);
  {
    std::ofstream probe(outputPath, std::ios::app);
    if (probe.good()) {
      // Some environments (e.g., Emscripten test runners) do not enforce POSIX permissions.
      GTEST_SKIP() << "Filesystem does not honor unwritable permission in this environment.";
    }
  }
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.preprocessToPath(testPath, outputPath), naja::verilog::VerilogException);

  std::filesystem::permissions(
    outputPath,
    std::filesystem::perms::owner_read
      | std::filesystem::perms::owner_write,
    std::filesystem::perm_options::add);
  std::filesystem::remove(outputPath);
}

TEST(NajaVerilogTestPreprocess, preprocessorKeywordEmptyReturnsFalse) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_empty_directive.v"));
  naja::verilog::VerilogPreprocessor preprocessor;
  EXPECT_THROW(preprocessor.preprocessFile(testPath), naja::verilog::VerilogException);
}

TEST(NajaVerilogTestPreprocess, blockCommentPassThrough) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_block_comment.v"));
  naja::verilog::VerilogPreprocessor preprocessor;
  auto output = preprocessor.preprocessFile(testPath);
  EXPECT_NE(std::string::npos, output.find("/* start comment `MACRO_SHOULD_NOT_EXPAND"));
  EXPECT_NE(std::string::npos, output.find("still comment */ assign o = i;"));
}

TEST(NajaVerilogTestPreprocess, includeAbsolutePathResolves) {
  auto tempDir = std::filesystem::temp_directory_path();
  auto absPath = tempDir / std::filesystem::path("naja_verilog_abs_include.v");
  auto includePath = tempDir / std::filesystem::path("naja_verilog_abs_test.v");
  {
    std::ofstream out(absPath);
    out << "module abs_inc(); endmodule\n";
  }
  {
    std::ofstream out(includePath);
    out << "`include \"" << absPath.string() << "\"\n";
    out << "module abs_top(); endmodule\n";
  }
  naja::verilog::VerilogPreprocessor preprocessor;
  auto output = preprocessor.preprocessFile(includePath);
  EXPECT_NE(std::string::npos, output.find("module abs_inc"));
  std::filesystem::remove(absPath);
  std::filesystem::remove(includePath);
}

TEST(NajaVerilogTestPreprocess, parsePathsWithoutPreprocess) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_plain_parse_paths.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(false);
  VerilogConstructorTest::Paths paths { testPath };
  constructor.parse(paths);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("plain_paths", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, parsePathsWithPreprocess) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_paths_with_macro.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  VerilogConstructorTest::Paths paths { testPath };
  constructor.parse(paths);

  ASSERT_EQ(1, constructor.modules_.size());
  EXPECT_EQ("paths_with_macro", constructor.modules_[0]->identifier_.name_);
}

TEST(NajaVerilogTestPreprocess, undefinedMacroPassesThrough) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_undefined_macro_passthrough.v"));
  naja::verilog::VerilogPreprocessor preprocessor;
  auto output = preprocessor.preprocessFile(testPath);
  EXPECT_NE(std::string::npos, output.find("`UNDEF_MACRO"));
}

TEST(NajaVerilogTestPreprocess, includeUnterminatedThrows) {
  std::filesystem::path testPath(
    std::filesystem::path(NAJA_VERILOG_BENCHMARKS)
    / std::filesystem::path("preprocess/preprocess_include_unterminated.v"));
  VerilogConstructorTest constructor;
  constructor.setPreprocessEnabled(true);
  EXPECT_THROW(constructor.parse(testPath), naja::verilog::VerilogException);
}

#include "args.h"
#include "gtest/gtest.h"

#include <string>
TEST(ArgsTest, NormalUsage) {
  const char *argv1[5] = {"progname", "-l", "log_file.log", "-h", "heap_dump.hprof"};
  ASSERT_NO_THROW(hprof_analyzer::parse_args(5, (char **) argv1));

  const char *argv2[] = {"progname", "-h", "heap_dump.hprof", "-l", "log_file.log"};
  ASSERT_NO_THROW(hprof_analyzer::parse_args(5, (char **) argv2));

  auto parsed1 = hprof_analyzer::parse_args(5, (char **) argv1);
  EXPECT_EQ(*parsed1.log, "log_file.log");
  EXPECT_EQ(*parsed1.heap_dump, "heap_dump.hprof");

  auto parsed2 = hprof_analyzer::parse_args(5, (char **) argv2);
  EXPECT_EQ(*parsed2.log, "log_file.log");
  EXPECT_EQ(*parsed2.heap_dump, "heap_dump.hprof");
}

TEST(ArgsTest, InvalidNumberOfArguments) {
  const char *argv1[] = {"progname", "-l", "log_file.log"};
  ASSERT_THROW(hprof_analyzer::parse_args(3, (char **) argv1), std::runtime_error);

  const char *argv2[] = {"progname", "-h", "heap_dump.hprof", "-l", "log_file.log", "extra"};
  ASSERT_THROW(hprof_analyzer::parse_args(6, (char **) argv2), std::runtime_error);
}

TEST(ArgsTest, InvalidArgumentOrder) {
  const char *argv[] = {"progname", "-l", "log_file.log", "extra", "heap_dump.hprof"};
  ASSERT_THROW(hprof_analyzer::parse_args(5, (char **) argv), std::runtime_error);

  const char *argv2[] = {"progname", "-l", "log_file.log", "extra", "-h", "heap_dump.hprof"};
  ASSERT_THROW(hprof_analyzer::parse_args(6, (char **) argv2), std::runtime_error);
}

TEST(ArgsTest, MissingArgument) {
  const char *argv[] = {"progname", "-l", "log_file.log"};
  ASSERT_THROW(hprof_analyzer::parse_args(3, (char **) argv), std::runtime_error);

  const char *argv2[] = {"progname", "-h", "heap_dump.hprof"};
  ASSERT_THROW(hprof_analyzer::parse_args(3, (char **) argv2), std::runtime_error);
}

TEST(ArgsTest, InvalidArgument) {
  const char *argv[] = {"progname", "-x", "invalid.log", "-y", "invalid.hprof"};
  ASSERT_THROW(hprof_analyzer::parse_args(5, (char **) argv), std::runtime_error);
}
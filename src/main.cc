#define CATCH_CONFIG_MAIN

#include <string_view>
#include <filesystem>
#include <catch2/catch_all.hpp>
#include "solution.h"

auto file(std::string_view const name) -> std::filesystem::path
{
  return std::filesystem::path(TEST_DATA_DIR) / name;
}

TEST_CASE("Test")
{
  auto solution = make_solution();

  REQUIRE(solution != nullptr);

  REQUIRE(solution->solve_from_file(file("1.txt"), file("1.txt")) == ISolution::Result::Equal);
  REQUIRE(solution->solve_from_file(file("1.txt"), file("2.txt")) == ISolution::Result::ASubsetOfB);
  REQUIRE(solution->solve_from_file(file("2.txt"), file("1.txt")) == ISolution::Result::BSubsetOfA);
  REQUIRE(solution->solve_from_file(file("1.txt"), file("3.txt")) == ISolution::Result::NotEqual);
  REQUIRE(solution->solve_from_file(file("3.txt"), file("1.txt")) == ISolution::Result::NotEqual);
  REQUIRE(solution->solve_from_file(file("1.txt"), file("4.txt")) == ISolution::Result::NotEqual);
  REQUIRE(solution->solve_from_file(file("4.txt"), file("1.txt")) == ISolution::Result::NotEqual);

  REQUIRE(solution->solve_from_file(file("2.txt"), file("4.txt")) == ISolution::Result::NotEqual);
  REQUIRE(solution->solve_from_file(file("4.txt"), file("2.txt")) == ISolution::Result::NotEqual);
}

// TEST_CASE("Test stdin")
// {
//   auto solution = make_solution();
//
//   REQUIRE(solution != nullptr);
//   REQUIRE(solution->solve_from_stdin() == ISolution::Result::Equal);
// }

// benchmark
TEST_CASE("Benchmark")
{
  auto const file1 = file("1.txt");
  auto const file2 = file("2.txt");
  auto solution = make_solution();

  REQUIRE(solution != nullptr);
  // clang-format off
  BENCHMARK("Benchmark") { return solution->solve_from_file(file1, file2); };
  // clang-format on
}

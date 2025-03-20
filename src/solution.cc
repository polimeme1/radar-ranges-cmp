#include "solution.h"

#include <iostream>
#include <set>
#include <type_traits>
#include <fstream>
#include <exception>
#include <algorithm>

#include <cmath>

#define SOLUTION_NAN_AWARE_COMPARISONS 1

struct Point
{
  double x;
  double y;
};

[[nodiscard]] static auto cmp_double_equal(double lhs, double rhs) -> bool
{
#if SOLUTION_NAN_AWARE_COMPARISONS
  if(std::isnan(lhs)) {
    return std::isnan(rhs) and std::signbit(lhs) == std::signbit(rhs);
  }
  return lhs == rhs;
#else
  return lhs == rhs;
#endif
}

[[nodiscard]] static auto cmp_double_less(double lhs, double rhs) -> bool
{
#if SOLUTION_NAN_AWARE_COMPARISONS
  if(std::isnan(lhs)) {
    if(std::isnan(rhs)) {
      return std::signbit(lhs) ? not std::signbit(rhs) : false;
    }
    return true;
  }
  return std::isnan(rhs) ? false : lhs < rhs;
#else
  return lhs < rhs;
#endif
}

[[nodiscard]] static auto cmp_points_equal(Point const& lhs, Point const& rhs) -> bool
{
  return cmp_double_equal(lhs.x, rhs.x) and cmp_double_equal(lhs.y, rhs.y);
}

[[nodiscard]] static auto cmp_points_less(Point const& lhs, Point const& rhs) -> bool
{
  return cmp_double_equal(lhs.x, rhs.x) ? cmp_double_less(lhs.y, rhs.y)
                                        : cmp_double_less(lhs.x, rhs.x);
}

using PointsSet = std::set<Point, std::decay_t<decltype(cmp_points_less)>>;

[[nodiscard]] static auto read_points_from_file(std::filesystem::path const& path) -> PointsSet
{
  PointsSet result {cmp_points_less};
  std::ifstream ifs {path};
  Point tmp {};
  while(ifs >> tmp.x >> tmp.y) {
    result.insert(tmp);
  }
  return result;
}

[[nodiscard]] static auto cmp_point_sets(PointsSet const& a, PointsSet const& b)
  -> ISolution::Result
{
  if(std::ranges::equal(a, b, cmp_points_equal)) {
    return ISolution::Result::Equal;
  }
  if(a.size() > b.size() and std::ranges::includes(a, b, cmp_points_less)) {
    return ISolution::Result::BSubsetOfA;
  }
  if(b.size() > a.size() and std::ranges::includes(b, a, cmp_points_less)) {
    return ISolution::Result::ASubsetOfB;
  }
  return ISolution::Result::NotEqual;
}

ISolution::~ISolution() = default;

class Solution final : public ISolution
{
 public:
  [[nodiscard]] auto solve_from_file(std::filesystem::path const& a, std::filesystem::path const& b)
    -> Result override;

  [[nodiscard]] auto solve_from_stdin() -> Result override;
};

auto Solution::solve_from_file(std::filesystem::path const& a, std::filesystem::path const& b)
  -> Result
try {
  auto const points_a = read_points_from_file(a);
  auto const points_b = read_points_from_file(b);
  return cmp_point_sets(points_a, points_b);
} catch(std::exception const& e) {
  std::cerr << "Solution::solve_from_file: " << e.what() << '\n';
  return Result::InternalError;
} catch(...) {
  std::cerr << "Solution::solve_from_file: Unknown error.\n";
  return Result::InternalError;
}

auto Solution::solve_from_stdin() -> Result { return Result::InternalError; }

auto make_solution() -> std::unique_ptr<ISolution> { return std::make_unique<Solution>(); }

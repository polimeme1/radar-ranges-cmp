#include "solution.h"

#include <iostream>
#include <set>
#include <type_traits>
#include <fstream>
#include <exception>
#include <algorithm>

#include <cmath>

/// \brief Необходимость учёта NaN значений при сравнении точек
/// \remark Можно вынести задание в CMake через target_compile_definitions
#define SOLUTION_NAN_AWARE_COMPARISONS 1

namespace
{
  /// \brief Точка на плоскости
  struct Point
  {
    double x;  ///< Положение точки по оси X
    double y;  ///< Положение точки по оси Y
  };
}  // namespace

/// \brief Сравнивает на равенство два double значения
/// \details
/// При SOLUTION_NAN_AWARE_COMPARISONS считает NaN == NaN и -NaN==-NaN.
/// \param lhs Сравниваемое значение
/// \param rhs Сравниваемое значение
/// \returns Признак равенства lhs и rhs
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

/// \brief Возвращает признак того, что первое double значение (lhs) меньше второго (rhs)
/// \details
/// При SOLUTION_NAN_AWARE_COMPARISONS считает NaN значения меньше не NaN значений,
/// а при сравнении двух NaN значений lhs считается меньше rhs, если lhs равен -NaN, а rhs +NaN.
/// \param lhs Первое сравниваемое значение (стоит слева от оператора \<)
/// \param rhs Второе сравниваемое значение (стоит справа от оператора \<)
/// \returns Если lhs меньше rhs, true, в остальных случаях - false
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

/// \brief Сравнивает две точки на равенство
/// \details
/// Две точки считаются равными, если положение первой точки на оси X равно положению второй точки
/// на оси X, как и положение первой точки на оси Y равно положению второй точки на оси Y
/// \param lhs Сравниваемая точка
/// \param rhs Сравниваемая точка
/// \returns Признак равенства точек
[[nodiscard]] static auto cmp_points_equal(Point const& lhs, Point const& rhs) -> bool
{
  return cmp_double_equal(lhs.x, rhs.x) and cmp_double_equal(lhs.y, rhs.y);
}

/// \brief Возвращает признак того, что первая точка (lhs) "меньше" второй точки (rhs)
/// \details
/// Точка lhs считается "меньше" точки rhs, если положение lhs на оси X меньше положения rhs на оси
/// X либо если при одинаковых положениях lhs и rhs на оси X положение lhs на оси Y меньше положения
/// rhs на оси Y.
/// Используется для сортировки точек в std::set и получения отсортированного множества уникальных
/// точек.
/// \param lhs Первая сравниваемая точка (стоит слева от оператора \<)
/// \param rhs Вторая сравниваемая точка (стоит справа от оператора \<)
/// \returns Если lhs "меньше" rhs, true, в остальных случаях - false
/// \see read_points_from_file
[[nodiscard]] static auto cmp_points_less(Point const& lhs, Point const& rhs) -> bool
{
  return cmp_double_equal(lhs.x, rhs.x) ? cmp_double_less(lhs.y, rhs.y)
                                        : cmp_double_less(lhs.x, rhs.x);
}

/// \brief Тип множества уникальных отсортированных точек
using PointsSet = std::set<Point, std::decay_t<decltype(cmp_points_less)>>;

/// \brief Считывает из файла точки и возвращает уникальные в виде отсортированного множества
/// \param path Путь до файла, из которого производится чтение
/// \returns Отсортированное множество уникальных считанных точек
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

/// \brief Определяет отношение двух отсортированных множеств уникальных точек на плоскости
/// \details
/// Возвращает ISolution::Result::Equal, если A содержит те же точки, что и B.
/// Возвращает ISolution::Result::BSubsetOfA, если B является подмножеством A.
/// Возвращает ISolution::Result::ASubsetOfB, если A является подмножеством B.
/// В остальных случаях возвращает ISolution::Result::NotEqual.
/// \param a Множество А
/// \param b Множество B
/// \returns Отношение множеств A и B
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

namespace
{
  class Solution final : public ISolution
  {
   public:
    [[nodiscard]] auto solve_from_file(
      std::filesystem::path const& a,
      std::filesystem::path const& b
    ) -> Result override;

    [[nodiscard]] auto solve_from_stdin() -> Result override;
  };
}  // namespace

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

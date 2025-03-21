#include "solution.h"

/// \brief Необходимость учёта NaN значений при сравнении точек
/// \remark Можно вынести задание в CMake через target_compile_definitions
#define SOLUTION_NAN_AWARE_COMPARISONS 1

#include <iostream>
#include <vector>
#include <fstream>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <ranges>
#include <string>
#include <ios>

#include <cctype>
#include <cstdlib>

#if SOLUTION_NAN_AWARE_COMPARISONS
#  include <cmath>
#endif

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

/// \brief Набор точек
using Points = std::vector<Point>;

/// \brief Считывает набор точек из файла
/// \param path Путь до файла, из которого производится чтение
/// \returns Набор считанных точек
[[nodiscard]] static auto read_points_from_file(std::filesystem::path const& path) -> Points
{
  Points result {};
  std::ifstream ifs {path};
  Point tmp {};
  while(ifs >> tmp.x >> tmp.y) {
    result.emplace_back(tmp);
  }
  return result;
}

/// \brief Возвращает признак того, что строка состоит только из пробельных символов
[[nodiscard]] static auto is_whitespace_line(std::string const& line) -> bool
{
  return std::ranges::all_of(line, [](char c) { return std::isspace(c); });
}

namespace
{
  /// \brief Результат извлечения double значения из строки
  struct ReadDoubleResult
  {
    double value;     ///< Полученное значение
    char const* end;  ///< Указатель на последний обработанный символ в строке
  };
}  // namespace

/// \brief Извлекает double значение из строки
/// \throws std::invalid_argument Не удалось произвести преобразование или результат выходит за
/// пределы представляемого типом double диапазона значений
[[nodiscard]] static auto read_double_from_line(char const* line_begin) -> ReadDoubleResult
{
  char* line_end;
  double const value = std::strtod(line_begin, &line_end);
  if(line_end == line_begin) {
    throw std::invalid_argument {"read_double_from_line: failed to parse double"};
  }
  if(value == HUGE_VAL) {
    throw std::invalid_argument {"read_double_from_line: value range error"};
  }
  return ReadDoubleResult {value, line_end};
}

/// \brief Выполняет чтение координат точки из строки
/// \returns Точка со считанными координатами
/// \throws std::invalid_argument При неудачном извлечении координат точки
/// \see read_double_from_line
[[nodiscard]] static auto read_point_from_line(std::string const& line) -> Point
{
  auto const [x, end] = read_double_from_line(line.c_str());
  auto const [y, _] = read_double_from_line(end);
  return {x, y};
}

/// \brief Выполняет чтение набора точек с потока ввода
/// \details
/// Чтение выполняется построчно. Конец ввода обозначается пустой строкой (или состоящей только из
/// пробельных символов). Ожидает, что каждая строка содержит последовательно записанные координаты
/// точки на оси X и Y.
/// \param is Читаемый поток ввода
/// \returns Считанный набор точек
/// \throws std::ios::failure При ошибке чтения очередной строки
/// \see is_whitespace_line
/// \see read_point_form_line
[[nodiscard]] static auto read_points_from_stream(std::istream& is) -> Points
{
  Points result {};
  std::string line;
  while(true) {
    if(not std::getline(is, line)) {
      throw std::ios::failure {"read_points_from_stream: stream reading error"};
    }
    if(is_whitespace_line(line)) {
      return result;
    }
    result.emplace_back(read_point_from_line(line));
  }
}

/// \brief Создаёт копию набора точек, в которой точки отсортированы cmp_points_less
[[nodiscard]] static auto make_sorted_copy_of_points(Points const& points) -> Points
{
  Points result = points;
  std::ranges::sort(result, cmp_points_less);
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
[[nodiscard]] static auto cmp_point_sets(Points const& a, Points const& b) -> ISolution::Result
{
  // получение отсортированных a и b
  Points sorted_a = make_sorted_copy_of_points(a);
  Points sorted_b = make_sorted_copy_of_points(b);

  // сдвиг всех дубликатов в конец отсортированных наборов точек
  auto const dup_a = std::ranges::unique(sorted_a, cmp_points_equal);
  auto const dup_b = std::ranges::unique(sorted_b, cmp_points_equal);

  // диапазоны отсортированных уникальных точек
  auto unique_a = std::ranges::subrange {std::ranges::begin(sorted_a), std::ranges::begin(dup_a)};
  auto unique_b = std::ranges::subrange {std::ranges::begin(sorted_b), std::ranges::begin(dup_b)};

  auto const len_a = std::ranges::size(unique_a);
  auto const len_b = std::ranges::size(unique_b);

  if(std::ranges::equal(unique_a, unique_b, cmp_points_equal)) {
    return ISolution::Result::Equal;
  }
  if(len_a > len_b and std::ranges::includes(unique_a, unique_b, cmp_points_less)) {
    return ISolution::Result::BSubsetOfA;
  }
  if(len_b > len_a and std::ranges::includes(unique_b, unique_a, cmp_points_less)) {
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

auto Solution::solve_from_stdin() -> Result
try {
  // считается, что std::cin ассоциирован с stdin
  auto const a = read_points_from_stream(std::cin);
  auto const b = read_points_from_stream(std::cin);
  return cmp_point_sets(a, b);
} catch(std::exception const& e) {
  std::cerr << "Solution::solve_from_stdin: " << e.what() << '\n';
  return Result::InternalError;
} catch(...) {
  std::cerr << "Solution::solve_from_stdin: Unknown error.\n";
  return Result::InternalError;
}

auto make_solution() -> std::unique_ptr<ISolution> { return std::make_unique<Solution>(); }

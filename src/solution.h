#pragma once

/**
 * Написать программу которая формирует два набора точек на плоскости
 * Источником данных могут быть:
 * - Входные файлы (примеры в приложении). В этом случае программе в качестве аргументов передаются
 *   пути к двум файлам.
 * - Ввод с клавиатуры. В этом случае пользователь должен ввести координаты точек вручную.
 *   Сравнивает полученные наборы и выдает результат этого сравнения:
 * - Наборы равны
 * - Один набор является подмножеством другого
 * - Наборы не равны
 *
 * Повторение точек не влияет на результат сравнения.
 */

#include <filesystem>
#include <memory>

class ISolution
{
 public:

  virtual ~ISolution();

  enum class Result
  {
    Equal,
    ASubsetOfB,
    BSubsetOfA,
    NotEqual,
    InternalError
  };

  [[nodiscard]] virtual auto solve_from_file(
    std::filesystem::path const& a,
    std::filesystem::path const& b
  ) -> Result = 0;

  [[nodiscard]] virtual auto solve_from_stdin() -> Result = 0;
};

[[nodiscard]] auto make_solution() -> std::unique_ptr<ISolution>;

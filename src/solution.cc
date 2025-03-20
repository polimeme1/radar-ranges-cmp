#include "solution.h"

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
{
  return Result::InternalError;
}

auto Solution::solve_from_stdin() -> Result { return Result::InternalError; }

auto make_solution() -> std::unique_ptr<ISolution> { return std::make_unique<Solution>(); }

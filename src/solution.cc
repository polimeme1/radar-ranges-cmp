#include "solution.h"

class Solution final : public ISolution
{
 public:
  [[nodiscard]] auto solve_from_file(
    std::filesystem::path const& lhs,
    std::filesystem::path const& rhs
  ) -> Result override;

  [[nodiscard]] auto solve_from_stdin() -> Result override;
};

auto Solution::solve_from_file(std::filesystem::path const& lhs, std::filesystem::path const& rhs)
  -> Result
{
  return Result::InternalError;
}

auto Solution::solve_from_stdin() -> Result { return Result::InternalError; }

auto make_solution() -> std::unique_ptr<ISolution> { return std::make_unique<Solution>(); }

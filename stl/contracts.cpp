#include "stl/contracts.h"

#include <iostream>

namespace rtw::stl::contracts
{

namespace
{

constexpr StringView to_string_view(const AssertionKind assertation_kind) noexcept
{
  switch (assertation_kind)
  {
  case AssertionKind::PRE:
    return "[pre]";
  case AssertionKind::POST:
    return "[post]";
  case AssertionKind::ASSERT:
    return "[assert]";
  default:
    return "unknown";
  }
}

constexpr StringView to_string_view(const EvaluationSemantic evaluation_semantic) noexcept
{
  switch (evaluation_semantic)
  {
  case EvaluationSemantic::IGNORE:
    return "ignore";
  case EvaluationSemantic::OBSERVE:
    return "observe";
  case EvaluationSemantic::ENFORCE:
    return "enforce";
  case EvaluationSemantic::QUICK_ENFORCE:
    return "quick_enforce";
  default:
    return "unknown";
  }
}

constexpr StringView to_string_view(const DetectionMode detection_mode) noexcept
{
  switch (detection_mode)
  {
  case DetectionMode::PREDICATE_FALSE:
    return "predicate_false";
  case DetectionMode::EVALUATION_EXCEPTION:
    return "evaluation_exception";
  default:
    return "unknown";
  }
}

} // namespace

namespace details
{

void handle_contract_violation(const EvaluationSemantic semantic, const DetectionMode detection_mode,
                               const StringView comment, const AssertionKind assertation_kind,
                               const SourceLocation source_location) noexcept
{
  const auto evaluation_exception =
      detection_mode == DetectionMode::EVALUATION_EXCEPTION ? std::current_exception() : nullptr;
  const ContractViolation violation{comment,          source_location, evaluation_exception,
                                    assertation_kind, semantic,        detection_mode};
  rtw::stl::contracts::handle_contract_violation(violation);
}

} // namespace details

void invoke_default_contract_violation_handler(const ContractViolation& violation) noexcept
{
  std::cerr << "Contract violation detected:\n";

  if (!violation.comment().empty())
  {
    std::cerr << "  Comment: " << violation.comment().data() << '\n';
  }

  std::cerr << "  Source: " << violation.source_location().file_name().data() << ':'
            << violation.source_location().line() << " in function "
            << violation.source_location().function_name().data() << '\n'
            << "  Assertion Kind: " << to_string_view(violation.assertation_kind()).data() << '\n'
            << "  Evaluation Semantic: " << to_string_view(violation.evaluation_semantic()).data() << '\n'
            << "  Detection Mode: " << to_string_view(violation.detection_mode()).data() << '\n';

  if (violation.evaluation_exception())
  {
    try
    {
      std::rethrow_exception(violation.evaluation_exception());
    }
    catch (const std::exception& ex)
    {
      std::cerr << "  Exception: " << ex.what() << "\n";
    }
    catch (...)
    {
      std::cerr << "  Exception: unknown\n";
    }
  }

  std::cerr.flush();

  if (violation.is_terminating())
  {
    std::terminate();
  }
}

void handle_contract_violation(const ContractViolation& violation) noexcept
{
  invoke_default_contract_violation_handler(violation);
}

} // namespace rtw::stl::contracts

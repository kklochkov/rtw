#pragma once

#include "stl/source_location.h"

#include <cstdint>
#include <exception>
#include <tuple>

namespace rtw::stl::contracts
{

/// Minimal C++26 contract implementation based on https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p2900r14.pdf
enum class AssertionKind : std::uint8_t
{
  PRE = 1,
  POST = 2,
  ASSERT = 3,
};

enum class EvaluationSemantic : std::uint8_t
{
  IGNORE = 1,
  OBSERVE = 2,
  ENFORCE = 3,
  QUICK_ENFORCE = 4,
};

enum class DetectionMode : std::uint8_t
{
  PREDICATE_FALSE = 1,
  EVALUATION_EXCEPTION = 2,
};

namespace details
{

void handle_contract_violation(const EvaluationSemantic semantic, const DetectionMode detection_mode,
                               const StringView comment, const AssertionKind assertion_kind,
                               const SourceLocation source_location) noexcept;

template <EvaluationSemantic SEMANTIC>
struct ContractViolationHandler
{
  static void handle_violation(const DetectionMode detection_mode, const StringView comment,
                               const AssertionKind assertion_kind, const SourceLocation source_location) noexcept
  {
    handle_contract_violation(SEMANTIC, detection_mode, comment, assertion_kind, source_location);
  }
};

template <>
struct ContractViolationHandler<EvaluationSemantic::QUICK_ENFORCE>
{
  static void handle_violation(const DetectionMode detection_mode, const StringView comment,
                               const AssertionKind assertion_kind, const SourceLocation source_location) noexcept
  {
    std::ignore = detection_mode;
    std::ignore = comment;
    std::ignore = assertion_kind;
    std::ignore = source_location;
    std::abort();
  }
};

} // namespace details

class ContractViolation
{
  friend void details::handle_contract_violation(const EvaluationSemantic semantic, const DetectionMode detection_mode,
                                                 const StringView comment, const AssertionKind assertion_kind,
                                                 const SourceLocation source_location) noexcept;

  ContractViolation(const StringView comment, const SourceLocation& source_location,
                    const std::exception_ptr& evaluation_exception, const AssertionKind assertion_kind,
                    const EvaluationSemantic evaluation_semantic, const DetectionMode detection_mode) noexcept
      : comment_{comment}, source_location_{source_location}, evaluation_exception_{evaluation_exception},
        assertion_kind_{assertion_kind}, evaluation_semantic_{evaluation_semantic}, detection_mode_{detection_mode}
  {
  }

public:
  StringView comment() const noexcept { return comment_; }
  SourceLocation source_location() const noexcept { return source_location_; }
  std::exception_ptr evaluation_exception() const noexcept { return evaluation_exception_; }
  AssertionKind assertion_kind() const noexcept { return assertion_kind_; }
  EvaluationSemantic evaluation_semantic() const noexcept { return evaluation_semantic_; }
  DetectionMode detection_mode() const noexcept { return detection_mode_; }
  bool is_terminating() const noexcept
  {
    return (evaluation_semantic_ == EvaluationSemantic::ENFORCE)
        || (evaluation_semantic_ == EvaluationSemantic::QUICK_ENFORCE);
  }

private:
  StringView comment_;
  SourceLocation source_location_;
  std::exception_ptr evaluation_exception_;
  AssertionKind assertion_kind_;
  EvaluationSemantic evaluation_semantic_;
  DetectionMode detection_mode_;
};

constexpr EvaluationSemantic current_semantic() noexcept
{
#if defined(RTW_CONTRACTS_IGNORE)
#pragma message("Contracts are ignored. Violations will not be detected or reported.")
  return EvaluationSemantic::IGNORE;
#elif defined(RTW_CONTRACTS_OBSERVE)
  return EvaluationSemantic::OBSERVE;
#elif defined(RTW_CONTRACTS_QUICK_ENFORCE)
  return EvaluationSemantic::QUICK_ENFORCE;
#else
  return EvaluationSemantic::ENFORCE;
#endif
}

void handle_contract_violation(const ContractViolation& violation) noexcept;
void invoke_default_contract_violation_handler(const ContractViolation& violation) noexcept;

template <typename PredicateT>
void check_predicate(const PredicateT& predicate, const StringView comment, const AssertionKind assertion_kind,
                     const SourceLocation source_location) noexcept
{
  constexpr auto SEMANTIC = current_semantic();
  if constexpr (SEMANTIC == EvaluationSemantic::IGNORE)
  {
    std::ignore = predicate;
    std::ignore = comment;
    std::ignore = assertion_kind;
    std::ignore = source_location;
  }
  else if constexpr ((SEMANTIC == EvaluationSemantic::OBSERVE) || (SEMANTIC == EvaluationSemantic::ENFORCE)
                     || (SEMANTIC == EvaluationSemantic::QUICK_ENFORCE))
  {
    bool violated = false;
    bool handled = false;
    try
    {
      violated = !predicate();
    }
    catch (...)
    {
      violated = true;
      details::ContractViolationHandler<SEMANTIC>::handle_violation(DetectionMode::EVALUATION_EXCEPTION, comment,
                                                                    assertion_kind, source_location);
      handled = true;
    }

    if (violated && !handled)
    {
      details::ContractViolationHandler<SEMANTIC>::handle_violation(DetectionMode::PREDICATE_FALSE, comment,
                                                                    assertion_kind, source_location);
    }
  }
}

template <typename PredicateT>
void pre(const PredicateT& predicate, const StringView comment,
         const SourceLocation source_location = SourceLocation::current()) noexcept
{
  check_predicate(predicate, comment, AssertionKind::PRE, source_location);
}

template <typename PredicateT>
void post(const PredicateT& predicate, const StringView comment,
          const SourceLocation source_location = SourceLocation::current()) noexcept
{
  check_predicate(predicate, comment, AssertionKind::POST, source_location);
}

template <typename PredicateT>
void contract_assert(const PredicateT& predicate, const StringView comment,
                     const SourceLocation source_location = SourceLocation::current()) noexcept
{
  check_predicate(predicate, comment, AssertionKind::ASSERT, source_location);
}

// NOLINTBEGIN(cppcoreguidelines-macro-usage,cppcoreguidelines-avoid-do-while)
#define RTW_CONTRACTS_SELECT_PREDICATE_MACRO(_1, _2, NAME, ...) NAME

#define RTW_CONTRACTS_CHECK_PREDICATE_3_ARGS(predicate, comment, verb)                                                 \
  do                                                                                                                   \
  {                                                                                                                    \
    rtw::stl::contracts::verb([&]() { return (predicate); }, comment);                                                 \
  }                                                                                                                    \
  while (false)

#define RTW_CONTRACTS_CHECK_PREDICATE_2_ARGS(predicate, verb) RTW_CONTRACTS_CHECK_PREDICATE_3_ARGS(predicate, "", verb)

#define RTW_PRE(...)                                                                                                   \
  RTW_CONTRACTS_SELECT_PREDICATE_MACRO(__VA_ARGS__, RTW_CONTRACTS_CHECK_PREDICATE_3_ARGS,                              \
                                       RTW_CONTRACTS_CHECK_PREDICATE_2_ARGS, STOP)(__VA_ARGS__, pre)

#define RTW_POST(...)                                                                                                  \
  RTW_CONTRACTS_SELECT_PREDICATE_MACRO(__VA_ARGS__, RTW_CONTRACTS_CHECK_PREDICATE_3_ARGS,                              \
                                       RTW_CONTRACTS_CHECK_PREDICATE_2_ARGS, STOP)(__VA_ARGS__, post)

#define RTW_ASSERT(...)                                                                                                \
  RTW_CONTRACTS_SELECT_PREDICATE_MACRO(__VA_ARGS__, RTW_CONTRACTS_CHECK_PREDICATE_3_ARGS,                              \
                                       RTW_CONTRACTS_CHECK_PREDICATE_2_ARGS, STOP)(__VA_ARGS__, contract_assert)
//  NOLINTEND(cppcoreguidelines-macro-usage,cppcoreguidelines-avoid-do-while)

} // namespace rtw::stl::contracts

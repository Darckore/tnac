//
// Settings for command types
//

#pragma once
#include "ast/ast_util.hpp"

namespace tnac_rt::commands
{
  //
  // Command id used for classification
  //
  enum class cmd_id : std::uint16_t
  {
#include "driver/cmd/command_ids.inl"
  };

  //
  // Classifies and verifies a command
  //
  class classifier
  {
  public:
    using value_type = const tnac::ast::command;
    using pointer    = value_type*;
    using reference  = value_type&;
    using ver_info   = value_type::verification_result;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(classifier);

    ~classifier() noexcept;
    
    classifier() noexcept;

  public:
    //
    // Checks a command and returns its id
    // If the command doesn't pass verification, returns Unknown
    //
    cmd_id verify(reference command) noexcept;

    //
    // Returns the result of the lates command verification
    //
    ver_info last_result() const noexcept;

  private:
    ver_info m_res{};
  };
}
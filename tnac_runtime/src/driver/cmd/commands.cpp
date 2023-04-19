#include "driver/cmd/commands.hpp"

namespace tnac_rt::commands
{
  // Classifier

  // Special members

  classifier::~classifier() noexcept = default;

  classifier::classifier() noexcept = default;

  // Public members

  cmd_id classifier::verify(reference command) noexcept
  {
    utils::unused(command);
    return cmd_id::Unknown;
  }

  classifier::ver_info classifier::last_result() const noexcept
  {
    return m_res;
  }
}
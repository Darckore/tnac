#include "eval/value/value_registry.hpp"

namespace tnac::eval
{
  // Special members

  registry::~registry() noexcept = default;

  registry::registry() noexcept = default;


  // Public members

  registry::tmp_val registry::consume() noexcept
  {
    auto res = fetch();
    return res;
  }

  value registry::value_for(entity_id id) noexcept
  {
    auto item = m_entityValues.find(id);
    if (item == m_entityValues.end())
      return {};

    return *(item->second);
  }

  value registry::evaluation_result() const noexcept
  {
    return *m_result;
  }


  // Private members

  registry::tmp_val registry::fetch() noexcept
  {
    tmp_val res;
    if (!m_inFlight.empty())
    {
      res = std::move(m_inFlight.top());
      m_inFlight.pop();
    }

    return res;
  }

}
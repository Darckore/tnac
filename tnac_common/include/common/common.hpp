#pragma once
#include <complex>
#include "utils/utils.hpp"

namespace tnac
{
  using buf_t    = std::string;
  using string_t = std::string_view;
  using char_t   = string_t::value_type;

  using file_data = std::expected<string_t, std::error_code>;
}


#define TNAC_BAD_PLATFORM static_assert(false, "Platform not supported");

#if _WIN64
#define TNAC_WINDOWS 1
#else
#endif

namespace tnac::rt
{
  using in_stream  = utils::istream;
  using out_stream = utils::ostream;
}

namespace tnac
{
  //
  // Helper object to facilitate easy casts from pointers to entity ids
  //
  class entity_id final
  {
  public:
    using id_t = std::uintptr_t;

  public:
    CLASS_SPECIALS_ALL_CUSTOM(entity_id);

    static consteval auto invalid_id() noexcept { return ~id_t{}; }

    constexpr entity_id() noexcept :
      m_value{ invalid_id() }
    {}

    constexpr entity_id(id_t id) noexcept :
      m_value{ id }
    {}

    entity_id(const void* ent) noexcept :
      entity_id{ reinterpret_cast<id_t>(ent) }
    {}

    constexpr entity_id(std::nullptr_t) noexcept :
      entity_id{ id_t{} }
    {}

    constexpr auto operator*() const noexcept
    {
      return m_value;
    }

    constexpr bool operator==(const entity_id&) const noexcept = default;

  private:
    id_t m_value{};
  };
}

template <>
struct std::hash<tnac::entity_id>
{
  constexpr auto operator()(const tnac::entity_id& id) const noexcept
  {
    return *id;
  }
};
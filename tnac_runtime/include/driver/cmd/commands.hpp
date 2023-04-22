//
// Commands
//

#pragma once

namespace tnac_rt::detail
{
  template <typename E>
  concept command_id = std::is_scoped_enum_v<E>;
}

namespace tnac_rt::commands
{
  //
  // Indicates whether or not a command passes verification
  //
  enum class verification : std::uint8_t
  {
    Correct,
    WrongName,
    TooFew,
    TooMany,
    WrongKind
  };

  //
  // Command descriptor
  //
  template <tnac_rt::detail::command_id ID>
  class descr
  {
  public:
    using id_type    = ID;
    using param_list = std::vector<tnac::tok_kind>;
    using size_type  = param_list::size_type;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(descr);

    ~descr() noexcept = default;

    descr(id_type id, param_list params, size_type req) noexcept :
      m_params{ std::move(params) },
      m_reqCount{ req },
      m_id{ id }
    {}

    descr(id_type id, param_list params) noexcept :
      descr{ id, std::move(params), size_type{} }
    {
      m_reqCount = m_params.size();
    }

    descr(id_type id) noexcept :
      descr{ id, param_list{}, size_type{} }
    {}

  private:
    param_list m_params;
    size_type m_reqCount{};
    id_type m_id;
  };


  //
  // Storage for commands
  // This is initied by whoever is interested in defining a custom command set
  //
  template <tnac_rt::detail::command_id ID>
  class store
  {
  public:
    using name_type = tnac::string_t;
    using id_type   = ID;
    using cmd_descr = descr<id_type>;
    using storage   = std::unordered_map<name_type, cmd_descr>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(store);

    ~store() noexcept = default;

    store() noexcept = default;

  public:
    //
    // Declares a command with the specified name and parameters
    // Subsequent additions overwrite the existing records
    //
    template <typename ...Args>
    void declare(name_type name, id_type id, Args&& ...args) noexcept
    {
      m_cmds.insert_or_assign(name, id, std::forward<Args>(args)...);
    }

  private:
    storage m_cmds;
  };
}
//
// IR base
//

#pragma once

namespace tnac::ir
{
  //
  // IR kinds
  //
  enum class ir_kind : std::uint8_t
  {
    Function,
    Arithmetic,
    Store,
    Load,
    Call,
    Jump,
    Ret
  };

  //
  // Base class for every IR node
  //
  class node
  {
  private:
    friend class builder;

  public:
    using kind = ir_kind;
    using enum ir_kind;

  public:
    CLASS_SPECIALS_NONE(node);

    virtual ~node() noexcept;

  protected:
    node(kind k) noexcept;

  public:
    //
    // Returns the node kind
    //
    kind what() const noexcept;

  private:
    kind m_kind;
  };

  inline auto get_id(const node& n) noexcept
  {
    return n.what();
  }

  //
  // Checks whether the target class inherits from ir::node
  //
  template <typename IR>
  concept ir_node = std::derived_from<IR, ir::node>;

}
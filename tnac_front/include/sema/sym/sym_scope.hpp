//
// Symbol
//

#pragma once

namespace tnac::semantics
{
  //
  // Represents a scope
  // We it need to keep track of where symbols are declared to disambiguate
  // between entities having the same name
  //
  class scope final
  {
  public:
    using depth_t = std::uint32_t;

  public:
    CLASS_SPECIALS_NONE(scope);

    ~scope() noexcept;

    scope(scope* encl) noexcept;

  public:
    //
    // Returns a pointer to the enclosing scope
    // 
    // const version
    //
    const scope* enclosing() const noexcept;

    //
    // Returns a pointer to the enclosing scope
    //
    scope* enclosing() noexcept;

    //
    // Returns the scope's depth
    //
    depth_t depth() const noexcept;

  private:
    scope* m_enclosing{};
    depth_t m_depth{};
  };
}
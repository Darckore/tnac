//
// Evaluation environment
//

#pragma once

namespace tnac::eval
{
  //
  // Maps entity ids to runtime indices
  //
  class env final
  {
  public:
    using mem_map = std::unordered_map<entity_id, entity_id>;
    using reg_opt = std::optional<entity_id>;

  public:
    CLASS_SPECIALS_NONE_CUSTOM(env);

    ~env() noexcept;

    env() noexcept;

  public:
    //
    // Adds a mapping
    //
    void map(entity_id ent, entity_id reg) noexcept;

    //
    // Finds a mapping
    //
    reg_opt find_reg(entity_id ent) const noexcept;

    //
    // Drops all data
    //
    void clear() noexcept;

  private:
    mem_map m_map;
  };
}
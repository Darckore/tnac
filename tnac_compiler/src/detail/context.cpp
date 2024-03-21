#include "compiler/detail/context.hpp"
#include "cfg/ir/ir.hpp"

namespace tnac::detail // var data
{
  struct context::var_data
  {
    CLASS_SPECIALS_ALL(var_data);

    ~var_data() noexcept = default;

    ir::vreg* m_reg{};
    ir::vreg* m_lastRead{};
    bool m_modified{};
  };
}


namespace tnac::detail // func data
{
  struct context::func_data
  {
    CLASS_SPECIALS_ALL(func_data);

    ~func_data() noexcept = default;

    ir::basic_block* m_curBlock{};
    ir::function* m_curFunction{};
    instr_iter m_funcFirst{};
    instr_iter m_funcLast{};
    ir::basic_block* m_terminal{};
    reg_idx m_regIdx{};
    symbol* m_lastStore{};
    var_store m_vars;
  };
}


namespace tnac::detail
{
  // Special members

  context::~context() noexcept = default;

  context::context() noexcept = default;


  // Public members

  void context::store(module_sym& sym, module_def& def) noexcept
  {
    [[maybe_unused]] auto newIt = m_data.try_emplace(&sym, &def);
    UTILS_ASSERT(newIt.second || newIt.first->second == &def);
  }

  context::module_def* context::locate(module_sym& sym) noexcept
  {
    auto found = m_data.find(&sym);
    return found != m_data.end() ? found->second : nullptr;
  }

  void context::store(symbol& sym, ir::vreg& reg) noexcept
  {
    auto&& fd = cur_data();
    auto newIt = fd.m_vars.try_emplace(&sym, var_data{});
    auto&& item = newIt.first->second;
    item.m_reg = &reg;
  }

  ir::vreg* context::locate(symbol& sym) noexcept
  {
    auto vd = locate_var(sym);
    return vd ? vd->m_reg : nullptr;
  }

  void context::wipe() noexcept
  {
    m_funcs.clear();
    m_data.clear();
  }

  void context::push(module_sym& sym) noexcept
  {
    m_stack.push_back(&sym);
  }

  context::module_sym* context::pop() noexcept
  {
    if (m_stack.empty())
      return {};

    auto sym = m_stack.back();
    m_stack.pop_back();
    return sym;
  }

  void context::enter_function(ir::function& fn) noexcept
  {
    instr_iter last{};
    if (auto owner = fn.owner_func())
      last = owner->entry().begin();
    if (!last && !m_funcs.empty())
      last = cur_data().m_funcLast;

    auto&& fd = m_funcs.emplace_back();
    fd.m_curFunction = &fn;
    fd.m_funcLast = last;
  }
  void context::exit_function() noexcept
  {
    m_funcs.pop_back();
  }
  ir::function& context::current_function() noexcept
  {
    auto&& fd = cur_data();
    UTILS_ASSERT(fd.m_curFunction);
    return *fd.m_curFunction;
  }

  ir::basic_block& context::create_block(string_t name) noexcept
  {
    return current_function().create_block(name);
  }

  void context::enter_block(ir::basic_block& block) noexcept
  {
    auto&& fd = cur_data();
    fd.m_curBlock = &block;
    for (auto&& var : fd.m_vars)
    {
      var.second.m_modified = true;
      var.second.m_lastRead = {};
    }
  }

  ir::basic_block& context::current_block() noexcept
  {
    auto&& fd = cur_data();
    UTILS_ASSERT(fd.m_curBlock);
    return *fd.m_curBlock;
  }

  ir::basic_block* context::terminal_block() noexcept
  {
    return cur_data().m_terminal;
  }

  ir::basic_block& context::terminal_or_entry() noexcept
  {
    auto&& fd = cur_data();
    return fd.m_terminal ? *fd.m_terminal : current_function().entry();
  }

  void context::terminate_at(ir::basic_block& term) noexcept
  {
    cur_data().m_terminal = &term;
  }

  void context::exit_block() noexcept
  {
    auto&& fd = cur_data();
    fd.m_curBlock = {};
  }

  void context::func_start_at(ir::instruction& instr) noexcept
  {
    auto&& fd = cur_data();
    if (!fd.m_funcFirst)
      fd.m_funcFirst = instr.to_iterator();
  }

  context::instr_iter context::funct_start() noexcept
  {
    auto&& fd = cur_data();
    if (fd.m_funcFirst)
      return fd.m_funcFirst;
    return fd.m_funcLast;
  }

  context::instr_iter context::func_end() noexcept
  {
    return cur_data().m_funcLast;
  }

  context::reg_idx context::register_index() noexcept
  {
    auto&& fd = cur_data();
    return fd.m_regIdx++;
  }

  void context::save_store(symbol& var) noexcept
  {
    cur_data().m_lastStore = &var;
  }

  void context::clear_store() noexcept
  {
    cur_data().m_lastStore = {};
  }

  context::symbol* context::last_store() noexcept
  {
    return cur_data().m_lastStore;
  }

  void context::read_into(symbol& var, ir::vreg& reg) noexcept
  {
    auto vd = locate_var(var);
    UTILS_ASSERT(vd);

    vd->m_lastRead = &reg;
    vd->m_modified = false;
  }

  void context::modify(symbol& var) noexcept
  {
    auto vd = locate_var(var);
    UTILS_ASSERT(vd);

    vd->m_lastRead = {};
    vd->m_modified = true;
  }

  ir::vreg* context::last_read(symbol& var) noexcept
  {
    auto vd = locate_var(var);
    UTILS_ASSERT(vd);
    return vd->m_lastRead;
  }

  context::instr_iter context::override_last(instr_iter newPos) noexcept
  {
    auto&& fd = cur_data();
    auto prev = fd.m_funcLast;
    fd.m_funcLast = newPos;
    return prev;
  }


  // Private members

  context::func_data& context::cur_data() noexcept
  {
    UTILS_ASSERT(!m_funcs.empty());
    return m_funcs.back();
  }

  context::var_data* context::locate_var(symbol& sym) noexcept
  {
    auto&& fd = cur_data();
    auto found = fd.m_vars.find(&sym);
    return found != fd.m_vars.end() ? &found->second : nullptr;
  }
}
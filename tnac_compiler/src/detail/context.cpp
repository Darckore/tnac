#include "compiler/detail/context.hpp"
#include "cfg/ir/ir.hpp"

namespace tnac::detail // func data
{
  struct context::func_data
  {
    CLASS_SPECIALS_ALL(func_data);

    ~func_data() noexcept = default;

    block_queue m_blocks;
    ir::function* m_curFunction{};
    instr_iter m_funcFirst{};
    ir::basic_block* m_terminal{};
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
    [[maybe_unused]] auto newIt = m_vars.try_emplace(&sym, &reg);
    UTILS_ASSERT(newIt.second || newIt.first->second == &reg);
  }

  ir::vreg* context::locate(symbol& sym) noexcept
  {
    auto found = m_vars.find(&sym);
    return found != m_vars.end() ? found->second : nullptr;
  }

  void context::wipe() noexcept
  {
    m_funcs.clear();
    m_data.clear();
    m_vars.clear();
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
    auto&& fd = m_funcs.emplace_back();
    fd.m_curFunction = &fn;
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

  void context::enqueue_block(ir::basic_block& block) noexcept
  {
    cur_data().m_blocks.push(&block);
  }

  ir::basic_block& context::current_block() noexcept
  {
    auto&& fd = cur_data();
    UTILS_ASSERT(!fd.m_blocks.empty());
    return *fd.m_blocks.front();
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
    UTILS_ASSERT(!fd.m_blocks.empty());
    fd.m_blocks.pop();
  }

  void context::func_start_at(ir::instruction& instr) noexcept
  {
    auto&& fd = cur_data();
    if (!fd.m_funcFirst)
      fd.m_funcFirst = instr.to_iterator();
  }

  context::instr_iter context::funct_start() noexcept
  {
    return cur_data().m_funcFirst;
  }


  // Private members

  context::func_data& context::cur_data() noexcept
  {
    UTILS_ASSERT(!m_funcs.empty());
    return m_funcs.back();
  }
}
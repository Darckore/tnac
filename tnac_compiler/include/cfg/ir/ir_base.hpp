//
// IR base
//

#pragma once

namespace tnac::ir
{
  class basic_block;
}

namespace tnac::ir
{
  //
  // IR kinds
  //
  enum class ir_kind : std::uint8_t
  {
    Function,
    Block,
    Edge,
    Instruction
  };

  //
  // Base class for every IR node
  //
  class node
  {
  public:
    using kind = ir_kind;
    using enum ir_kind;

  public:
    CLASS_SPECIALS_NONE(node);

    virtual ~node() noexcept;

    node(kind k) noexcept;

  public:
    //
    // Returns the node kind
    //
    kind what() const noexcept;

  private:
    kind m_kind;
  };

  //
  // Checks whether the target class inherits from ir::node
  //
  template <typename IR>
  concept ir_node = std::derived_from<IR, ir::node>;

  inline auto get_id(const ir_node auto& n) noexcept
  {
    return n.what();
  }
}

namespace tnac::ir::detail
{
  //
  // An iterator for an associative collection of IR nodes
  //
  template <typename Key, ir_node Node>
  class ir_iterator final
  {
  public:
    using key_type     = Key;
    using value_type   = Node;
    using underlying_t = std::unordered_map<key_type, value_type>;
    using iter         = underlying_t::iterator;
    using size_type    = underlying_t::size_type;

  public:
    CLASS_SPECIALS_NODEFAULT(ir_iterator);

    ~ir_iterator() noexcept = default;

    explicit ir_iterator(iter it) noexcept :
      m_iter{ it }
    {}

    bool operator==(const ir_iterator&) const noexcept = default;

    auto operator++() noexcept
    {
      ++m_iter;
      return *this;
    }
    auto operator++(int) noexcept
    {
      auto self = *this;
      ++(*this);
      return self;
    }

    value_type& operator*() noexcept
    {
      return m_iter->second;
    }

    auto operator->() noexcept
    {
      return &(operator*());
    }

    auto key() const noexcept
    {
      return m_iter->first;
    }

    iter get() noexcept
    {
      return m_iter;
    }

  private:
    iter m_iter;
  };


  //
  // A proxy associative container for a collection of IR nodes
  //
  template <typename Key, ir_node Node>
  class ir_container final
  {
  public:
    using key_type        = Key;
    using value_type      = Node;
    using iterator        = ir_iterator<key_type, value_type>;
    using underlying_t    = iterator::underlying_t;
    using pointer         = underlying_t*;
    using reference       = underlying_t&;
    using const_pointer   = const underlying_t*;
    using const_reference = const underlying_t&;

  public:
    CLASS_SPECIALS_NODEFAULT_NOCOPY(ir_container);

    ~ir_container() noexcept = default;

    explicit ir_container(reference value) noexcept :
      m_value{ &value }
    {}

  public:
    //
    // Locates a node by the associated key
    // 
    // const version
    //
    const value_type* find(const key_type& key) const noexcept
    {
      auto found = m_value->find(key);
      return found != m_value->end() ? &found->second : nullptr;
    }

    //
    // Locates a node by the associated key
    //
    value_type* find(const key_type& key) noexcept
    {
      return FROM_CONST(find, key);
    }

    //
    // Removes the specified node
    //
    void remove(const key_type& key) noexcept
    {
      m_value->erase(key);
    }

    //
    // Removes the node pointed to by an iterator
    //
    void remove(iterator it) noexcept
    {
      m_value->erase(it.get());
    }

    //
    // Adds a node
    //
    template <typename ...Args>
    value_type& add(const key_type& key, Args&& ...args) noexcept
    {
      auto newItem = m_value->try_emplace(key, std::forward<Args>(args)...);
      UTILS_ASSERT(newItem.second);
      return newItem.first->second;
    }

  public:
    auto begin() const noexcept
    {
      return iterator{ m_value->begin() };
    }
    auto begin() noexcept
    {
      return iterator{ m_value->begin() };
    }
    auto end() const noexcept
    {
      return iterator{ m_value->end() };
    }
    auto end() noexcept
    {
      return iterator{ m_value->end() };
    }

  private:
    pointer m_value{};
  };
}
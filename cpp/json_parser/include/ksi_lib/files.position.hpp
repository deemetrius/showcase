#pragma once

namespace ksi::files {

  using index_t = std::intptr_t;

  class position
  {
  protected:
    struct inner_type
    {
      index_t count_no_tabs;
      index_t all_tab_spaces;

      // actions:

      constexpr void reset()
      {
        this->count_no_tabs = 0;
        this->all_tab_spaces = 0;
      }

      constexpr index_t total() const
      {
        return (this->count_no_tabs + this->all_tab_spaces);
      }

      constexpr index_t tail(index_t tab_size) const
      {
        return (this->total() % tab_size);
      }

      constexpr void add_tab(index_t tab_size)
      {
        this->all_tab_spaces += ( tab_size - this->tail(tab_size) );
      }
    };

  public:
    struct data_type
    {
      index_t char_pos;
      index_t line;
      index_t column; // calcultated

      // actions:

      constexpr void update(inner_type from)
      {
        this->column = (from.total() + 1);
      }
    };

  protected:
    index_t tab_size{4};
    inner_type line_info{0, 0};
    data_type pos{0, 1, -1};
    bool was_cr{false};

  public:
    constexpr data_type const & operator -> ()
    {
      this->pos.update(line_info);
      return this->pos;
    }

  protected:
    constexpr void add_line()
    {
      ++this->pos.line;
      this->line_info.reset();
    }

    constexpr void on_cr()
    {
      this->was_cr = true;
      this->add_line();
    }

    constexpr void on_lf()
    {
      if( this->was_cr == false )
      {
        this->add_line();
        return;
      }
      this->was_cr = false;
    }

    constexpr void on_tab()
    {
      this->was_cr = false;
      this->line_info.add_tab(this->tab_size);
    }

    constexpr void on_default()
    {
      this->was_cr = false;
      ++this->line_info.count_no_tabs;
    }

  public:
    position(index_t arg_tab_size)
      : tab_size{ arg_tab_size }
    {}

    template <typename Char>
    void recognized(Char ch)
    {
      ++this->pos.char_pos;
      using info = ksi::chars::info<Char>;
      switch( ch )
      {
        case info::cr: 
        { this->on_cr(); break; }

        case info::lf:
        { this->on_lf(); break; }

        case info::tab:
        { this->on_tab(); break; }

        default:
        { this->on_default(); break; }
      }
    }
  };

} // end ns

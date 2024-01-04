#pragma once

namespace ksi::files {

  using index_t = std::intptr_t;

  class position
  {
    struct data_type
    {
      index_t line;
      index_t column;
      index_t char_pos;
    };
  protected:
    index_t m_tab_size{4};
    index_t m_tab_spaces{0};
    index_t m_line_size{0}; // without tabs
    bool m_was_cr{false};
    data_type data{1, 1, 0}; // calculated

  public:
    constexpr data_type const & operator -> ()
    {
      data.column = (m_line_size + m_tab_spaces + 1);
      return data;
    }

  protected:
    constexpr void add_line()
    {
      data.line += 1;
      m_line_size = 0;
      m_tab_spaces = 0;
    }

    constexpr void on_cr()
    {
      m_was_cr = true;
      add_line();
    }

    constexpr void on_lf()
    {
      if( m_was_cr == false )
      {
        add_line();
        return;
      }
      m_was_cr = false;
    }

    constexpr void on_tab()
    {
      m_was_cr = false;
      index_t line_pos{ m_line_size + m_tab_spaces };
      index_t tail{ line_pos % m_tab_size };
      m_tab_spaces += (m_tab_size - tail);
    }

    constexpr void on_default()
    {
      m_was_cr = false;
      ++m_line_size;
    }

  public:
    position(index_t tab_size)
      : m_tab_size{ tab_size }
    {}

    template <typename Char>
    void on_char(Char ch)
    {
      ++data.char_pos;
      using info = ksi::chars::info<Char>;
      switch( ch )
      {
        case info::cr: 
        { on_cr(); break; }

        case info::lf:
        { on_lf(); break; }

        case info::tab:
        { on_tab(); break; }

        default:
        { on_default(); break; }
      }
    }
  };

} // end ns

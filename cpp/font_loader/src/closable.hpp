#pragma once

namespace lib {

  template <typename Handle>
  struct closable
  {
    using handle_type = Handle;

    static handle_type init();

    static void no_close(handle_type &) {}
    static void close(handle_type &);

    using closer_type = decltype(&no_close);

    handle_type handle;
    closer_type fn_close{ &no_close };

    closable()
      : handle{ init() }
    {}

    closable(handle_type h, closer_type closer_fn = &close)
      : handle{ h }
      , fn_close{ &close }
    {}

    ~closable()
    {
      fn_close(handle);
    }

    // no copy
    closable(const closable &) = delete;
    // no copy assign
    closable & operator = (const closable &) = delete;

    // move
    closable(closable && other)
      : handle{ other.handle }
      , fn_close{ other.fn_close }
    {
      other.release();
    }

    // move assign
    closable & operator = (closable && other)
    {
      handle = other.handle;
      fn_close = other.fn_close;
      other.release();
      return *this;
    }

    void release()
    {
      handle = init();
      fn_close = &no_close;
    }

    bool not_ready() const
    {
      return (fn_close == &no_close);
    }
  };

  template <typename Handle>
  inline Handle closable<Handle>::init()
  {
    return nullptr;
  }

  using closable_file = closable<FILE *>;
  using closable_font = closable<HFONT>;
  using closable_font_res = closable<HANDLE>;

  template <>
  inline void closable_file::close(handle_type & handle)
  {
    if (handle == nullptr) { return; }
    fclose(handle);
    handle = nullptr;
  }

  template <>
  inline void closable_font::close(handle_type& handle)
  {
    if (handle == nullptr) { return; }
    DeleteObject(handle);
    handle = nullptr;
  }

  template <>
  inline void closable_font_res::close(handle_type & handle)
  {
    if (handle == nullptr) { return; }
    RemoveFontMemResourceEx(handle);
    handle = nullptr;
  }

} // end ns

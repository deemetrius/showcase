#pragma once

namespace ksi::lib {

  template <typename Char>
  struct reader
  {
    virtual bool is_end() const = 0;
    virtual Char read_char() = 0;
  };

  template <typename String>
  struct string_reader : public reader<typename String::value_type>
  {
    using text = const String;
    using iterator = String::const_iterator;
    using char_type = String::value_type;
    using char_info = ksi::chars::info<char_type>;

    text string;
    iterator it{string.cbegin()};

    string_reader(text str)
      : string(str)
    {}

    bool is_end() const override
    {
      return (string.cend() == it);
    }

    char_type read_char() override
    {
      if( is_end() ) { return char_info::null; }
      char_type ret{*it};
      ++it;
      return ret;
    }
  };

} // end ns

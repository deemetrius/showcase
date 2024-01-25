#pragma once

#include <string>

namespace ksi::conv {

  struct string_cast
  {
    template <typename Result>
    struct to;
  };


  template <>
  struct string_cast::to<std::wstring>
  {
    using result_type = std::wstring;

    template <std::integral Number>
    result_type operator () (Number param) const
    {
      return std::to_wstring(param);
    }

    result_type operator () (std::wstring param) const
    {
      return param;
    }

    result_type operator () (std::string param) const
    {
      std::mbstate_t state = std::mbstate_t();

      const char * src = param.c_str();
      std::size_t len{ 0 };
      if( mbsrtowcs_s(&len, nullptr, 0, &src, param.size(), &state) )
      {
        return {};
      }

      std::wstring ret{ len - 1, L'\0', std::allocator<wchar_t>{} };
      if( mbsrtowcs_s(nullptr, ret.data(), len, &src, param.size(), &state) )
      {
        return {};
      }

      return ret;
    }
  };


  template <>
  struct string_cast::to<std::string>
  {
    using result_type = std::string;

    template <std::integral Number>
    result_type operator () (Number param) const
    {
      return std::to_string(param);
    }

    result_type operator () (std::string param) const
    {
      return param;
    }

    result_type operator () (std::wstring param) const
    {
      std::mbstate_t state = std::mbstate_t();

      const wchar_t * src = param.c_str();
      std::size_t len{ 0 };
      if( wcsrtombs_s(&len, nullptr, 0, &src, param.size(), &state) )
      {
        return {};
      }

      std::string ret{ len - 1, '\0', std::allocator<char>{} };
      if( wcsrtombs_s(nullptr, ret.data(), len, &src, param.size(), &state) )
      {
        return {};
      }

      return ret;
    }
  };

} // end ns

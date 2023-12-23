#pragma once

namespace lib {

  using byte = std::uint8_t;
  using text = std::string;
  using text_pass = const text &;
  using index_t = std::ptrdiff_t;
  using size_t = std::size_t;


  struct error
  {
    text message;
  };

  struct error_file
    : public error
  {
    text path;
  };


  enum class file_type { n_unknown, n_font };


  struct file
  {
    using data_type = std::vector<byte>;

    text path;
    file_type type;
    data_type bytes;
    index_t index;

    static closable_file open_read(text_pass path)
    {
      FILE * fp{nullptr};
      if( fopen_s(&fp, path.c_str(), "rb") != 0 )
      {
        return {};
      }
      return { fp };
    }

    static data_type read(text_pass path)
    {
      closable_file fc = open_read(path);
      if( fc.not_ready() )
      {
        throw error_file{ "Can't open file for reading: " + path, path };
      }
      if( fseek(fc.handle, 0, SEEK_END) != 0 )
      {
        throw error_file{ "Can't obtain file size: " + path, path };
      }
      size_t size = static_cast<size_t>( ftell(fc.handle) );
      if( size < 0 )
      {
        throw error_file{ "Can't obtain file size: " + path, path };
      }
      if( fseek(fc.handle, 0, SEEK_SET) != 0 )
      {
        throw error_file{ "Can't obtain file size: " + path, path };
      }
      data_type ret(size, 0);
      size_t size_read = fread(ret.data(), sizeof(byte), size, fc.handle);
      if( size_read != size )
      {
        throw error_file{ "Can't read file: " + path, path };
      }
      return ret;
    }
  };


  struct file_manager
  {
    using files_vec = std::vector<file>;
    using files_map = std::map<text, index_t>;

    files_vec files;
    files_map loaded;

    index_t index_of(text_pass path) const
    {
      typename files_map::const_iterator it = loaded.find(path);
      return (( it == std::as_const(loaded).end() ) ? -1 : it->second );
    }

    index_t load_file(text_pass path, file_type type)
    {
      // check if already loaded
      index_t index = index_of(path);
      if( index >= 0 )
      {
        // update type
        files[index].type = type;
        return index;
      }

      index = files.size();
      files.emplace_back(file{ path, type, file::read(path), index });
      loaded.try_emplace(path, index);
      return index;
    }
  };

} // end ns

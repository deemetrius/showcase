#pragma once

namespace lib {

  struct res_font
  {
    closable_font_res res;
    index_t font_index;
    index_t file_index;
    index_t count;

    /*static void close_memory_loaded_font(closable_font_res::handle_type & handle)
    {
      RemoveFontMemResourceEx(handle);
      handle = nullptr;
    }*/
  };

  struct res_manager
  {
    using font_vec = std::vector<res_font>;
    using font_map = std::map<index_t, index_t>; // file-id : font-id

    file_manager file_man;
    font_vec fonts;
    font_map fonts_loaded;

    HANDLE font_get(index_t font_index)
    {
      if( font_index < 0 || static_cast<size_t>(font_index) >= fonts.size() )
      {
        return nullptr;
      }
      return static_cast<HFONT>(fonts[font_index].res.handle);
    }

    index_t font_index_of(index_t file_index) const
    {
      typename font_map::const_iterator it = fonts_loaded.find(file_index);
      return ((it == fonts_loaded.end()) ? -1 : it->second );
    }

    index_t font_load(text_pass path)
    {
      index_t file_index = file_man.load_file(path, file_type::n_font);
      index_t font_index = font_index_of(file_index);
      if( font_index >= 0 )
      {
        return font_index;
      }

      file * hfile = &file_man.files[file_index];
      DWORD fonts_count{ 0 };
      HANDLE handle = AddFontMemResourceEx(
        hfile->bytes.data(),
        static_cast<DWORD>( hfile->bytes.size() ),
        0,
        &fonts_count
      );
      if( handle == nullptr )
      {
        throw error{ "Fail to add font from memory data." };
      }
      font_index = fonts.size();
      fonts.emplace_back(
        res_font{
          handle,
          file_index,
          fonts_count
        }
      );
      fonts_loaded.try_emplace(file_index, font_index);
      return font_index;
    }
  };

} // end ns

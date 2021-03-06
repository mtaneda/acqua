#pragma once

/*!
  acqua library

  Copyright (c) 2016 Haruhiko Uchida
  The software is released under the MIT license.
  http://opensource.org/licenses/mit-license.php
 */

#include <string>

namespace acqua { namespace email {

/*!
  beg から end までのメールヘッダーのパラメータを str に変換する.
  beg から end までは RFC2047 エンコードされている場合にはデコードされる。
*/
template <typename It, typename CharT>
void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str);

/*!
  beg から end までのメールヘッダーのパラメータを str と params に変換する.
  beg から end もしくは文字区切り(改行 or 空白 or ;)までは str にデコードされ、
  それ以降は、key=value 形式で params にデコードされる。
*/
template <typename It, typename CharT, typename Params>
void decode_mimeheader(It beg, It end, std::basic_string<CharT> & str, Params & params);

} }

#include <acqua/email/decode_mimeheader.ipp>

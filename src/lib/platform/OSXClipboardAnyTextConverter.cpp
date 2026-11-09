/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2004 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "platform/OSXClipboardAnyTextConverter.h"

#include <algorithm>

//
// OSXClipboardAnyTextConverter
//

IClipboard::Format OSXClipboardAnyTextConverter::getFormat() const
{
  return IClipboard::Format::Text;
}

std::string OSXClipboardAnyTextConverter::fromIClipboard(const std::string &data) const
{
  // convert to desired encoding; line breaks are left as-is (LF)
  return doFromIClipboard(data);
}

std::string OSXClipboardAnyTextConverter::toIClipboard(const std::string &data) const
{
  // convert text then newlines
  return convertLinefeedToUnix(doToIClipboard(data));
}

std::string OSXClipboardAnyTextConverter::convertLinefeedToUnix(const std::string &src)
{
  // note -- we assume src is a valid UTF-8 string
  // normalize fully to unix line breaks: "\r\n" becomes "\n" and a lone
  // "\r" also becomes "\n", so mac style line endings never double up
  std::string dst;
  dst.reserve(src.size());
  const size_t n = src.size();
  for (size_t i = 0; i < n; ++i) {
    if (src[i] == '\r') {
      if (i + 1 >= n || src[i + 1] != '\n') {
        // lone carriage return becomes a newline
        dst += '\n';
      }
      // else: "\r\n" pair, drop the '\r' and keep the following '\n'
    } else {
      dst += src[i];
    }
  }

  return dst;
}

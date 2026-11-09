/*
 * Deskflow -- mouse and keyboard sharing utility
 * SPDX-FileCopyrightText: (C) 2012 - 2016 Synergy App Ltd
 * SPDX-FileCopyrightText: (C) 2002 Chris Schoeneman
 * SPDX-License-Identifier: GPL-2.0-only WITH LicenseRef-OpenSSL-Exception
 */

#include "platform/MSWindowsClipboardAnyTextConverter.h"

//
// MSWindowsClipboardAnyTextConverter
//

IClipboard::Format MSWindowsClipboardAnyTextConverter::getFormat() const
{
  return IClipboard::Format::Text;
}

HANDLE
MSWindowsClipboardAnyTextConverter::fromIClipboard(const std::string &data) const
{
  // convert linefeeds and then convert to desired encoding
  std::string text = doFromIClipboard(convertLinefeedToWin32(data));
  uint32_t size = (uint32_t)text.size();

  // copy to memory handle
  HGLOBAL gData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, size);
  if (gData != nullptr) {
    // get a pointer to the allocated memory
    char *dst = (char *)GlobalLock(gData);
    if (dst != nullptr) {
      memcpy(dst, text.data(), size);
      GlobalUnlock(gData);
    } else {
      GlobalFree(gData);
      gData = nullptr;
    }
  }

  return gData;
}

std::string MSWindowsClipboardAnyTextConverter::toIClipboard(HANDLE data) const
{
  // get datator
  const char *src = (const char *)GlobalLock(data);
  uint32_t srcSize = (uint32_t)GlobalSize(data);
  if (src == nullptr || srcSize <= 1) {
    return std::string();
  }

  // convert text
  std::string text = doToIClipboard(std::string(src, srcSize));

  // release handle
  GlobalUnlock(data);

  // convert newlines
  return convertLinefeedToUnix(text);
}

std::string MSWindowsClipboardAnyTextConverter::convertLinefeedToWin32(const std::string &src) const
{
  // note -- we assume src is a valid UTF-8 string

  // count newlines in string
  uint32_t numNewlines = 0;
  uint32_t n = (uint32_t)src.size();
  for (const char *scan = src.c_str(); n > 0; ++scan, --n) {
    if (*scan == '\n') {
      ++numNewlines;
    }
  }
  if (numNewlines == 0) {
    return src;
  }

  // allocate new string
  std::string dst;
  dst.reserve(src.size() + numNewlines);

  // copy string, converting newlines
  // do not insert a carriage return if the previous character is already one,
  // so that an already canonical "\r\n" is not turned into "\r\r\n"
  n = (uint32_t)src.size();
  char prev = '\0';
  for (const char *scan = src.c_str(); n > 0; ++scan, --n) {
    if (scan[0] == '\n' && prev != '\r') {
      dst += '\r';
    }
    dst += scan[0];
    prev = scan[0];
  }

  return dst;
}

std::string MSWindowsClipboardAnyTextConverter::convertLinefeedToUnix(const std::string &src) const
{
  // count line breaks in string (pairs and lone carriage returns)
  uint32_t numNewlines = 0;
  uint32_t n = (uint32_t)src.size();
  for (const char *scan = src.c_str(); n > 0; ++scan, --n) {
    if (scan[0] == '\r') {
      ++numNewlines;
    }
  }
  if (numNewlines == 0) {
    return src;
  }

  // allocate new string
  std::string dst;
  dst.reserve(src.size());

  // copy string, converting newlines; normalize fully to unix line breaks:
  // "\r\n" becomes "\n" and a lone "\r" also becomes "\n"
  n = (uint32_t)src.size();
  for (const char *scan = src.c_str(); n > 0; ++scan, --n) {
    if (scan[0] == '\r') {
      if (scan[1] != '\n') {
        dst += '\n';
      }
    } else {
      dst += scan[0];
    }
  }

  return dst;
}

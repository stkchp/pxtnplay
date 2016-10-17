#include <algorithm>
#include <fstream>
#include <string>

#include "./pxwrDoc.h"

pxwrDoc::pxwrDoc() { offset = 0; }
pxwrDoc::~pxwrDoc() {}
bool pxwrDoc::Open_path(const char *path, const char *mode)
{
  // simple mode check
  std::string _mode(mode);
  if (_mode.find('w') != std::string::npos) return false;  // read only support

  // read data upto PXWRDOC_MAXSIZE
  try {
    std::ifstream ifs(path, std::ios::binary);
    data.resize(PXWRDOC_MAXSIZE + 1);
    ifs.read(data.data(), data.size());
    auto readed = ifs.gcount();
    data.resize(readed);
    if (!ifs.eof()) data.clear();

  } catch (...) {
    data.clear();
  }

  // release unused memory
  data.shrink_to_fit();

  return data.size() ? true : false;
}

void pxwrDoc::Close()
{
  offset = 0;
  data.clear();
  data.shrink_to_fit();
}

bool pxwrDoc::Seek(s32 mode, s32 val)
{
  std::int64_t w_pos = 0;

  switch (mode) {
  case SEEK_SET:
    w_pos = val;
    break;
  case SEEK_CUR:
    w_pos = offset + val;
    break;
  case SEEK_END:
    w_pos = data.size() - val;
    break;
  default:
    return false;
  }

  if (w_pos >= data.size()) return false;
  if (w_pos < 0) return false;

  offset = w_pos;
  return true;
}

bool pxwrDoc::r(void *p, s32 size, s32 num)
{
  if (data.size() == 0) return false;

  for (s32 i = 0; i < num; i++) {
    if (offset + size > data.size()) return false;
    std::copy_n(data.begin() + offset, size, &((char *)p)[i]);
    offset += size;
  }
  return true;
}

s32 pxwrDoc_v_chk(s32 val)
{
  u32 us;

  us = (u32)val;
  // 1byte(7bit)
  if (us < 0x80) return 1;
  // 2byte(14bit)
  if (us < 0x4000) return 2;
  // 3byte(21bit)
  if (us < 0x200000) return 3;
  // 4byte(28bit)
  if (us < 0x10000000) return 4;
  return 5;
}

bool pxwrDoc::v_r(s32 *p)
{
  if (data.size() == 0) return false;

  size_t i = 0;
  u8 a[5] = {0};
  u8 b[5] = {0};

  for (i = 0; i < 5; i++) {
    if (!pxwrDoc::r(&a[i], 1, 1)) return false;
    if (!(a[i] & 0x80)) break;
  }
  switch (i) {
  case 0:
    b[0] = ((a[0] & 0x7F) >> 0);
    break;
  case 1:
    b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
    b[1] = ((a[1] & 0x7F) >> 1);
    break;
  case 2:
    b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
    b[1] = ((a[1] & 0x7F) >> 1) | (a[2] << 6);
    b[2] = ((a[2] & 0x7F) >> 2);
    break;
  case 3:
    b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
    b[1] = ((a[1] & 0x7F) >> 1) | (a[2] << 6);
    b[2] = ((a[2] & 0x7F) >> 2) | (a[3] << 5);
    b[3] = ((a[3] & 0x7F) >> 3);
    break;
  case 4:
    b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
    b[1] = ((a[1] & 0x7F) >> 1) | (a[2] << 6);
    b[2] = ((a[2] & 0x7F) >> 2) | (a[3] << 5);
    b[3] = ((a[3] & 0x7F) >> 3) | (a[4] << 4);
    b[4] = ((a[4] & 0x7F) >> 4);
    break;
  case 5:
    return false;
  }

  *p = *((s32 *)b);

  return true;
}

s32 pxwrDoc::FileSize() const { return data.size(); }
bool pxwrDoc::w(const void *p, s32 size, s32 num) { return false; }
s32 pxwrDoc::v_w(s32 val, s32 *p_add) { return 0; }

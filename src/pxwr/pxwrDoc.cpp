#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "./pxwrDoc.h"

pxwrDoc::pxwrDoc()
{
	_p = NULL;
	_len = 0;
	_b_file = false;
	_ofs = 0;
}

pxwrDoc::~pxwrDoc()
{
	if (_b_file && _p)
		fclose((FILE *)_p);
}

s32 pxwrDoc::FileSize() const { return _len; }

bool pxwrDoc::Open_path(const char *path, const char *mode)
{
	bool b_ret = false;

	// TODO: write code to open file in linux.
	// _p = (char*)pxwFile_Document_Open( path, mode, &_len );

	if (!_p)
		goto End;

	_ofs = 0;
	_b_file = true;
	b_ret = true;
End:
	return b_ret;
}

bool pxwrDoc::SetRead(void *p, s32 len)
{
	if (!p || len < 1)
		return false;

	_p = (char *)p;
	_len = len;
	_b_file = false;
	_ofs = 0;

	return true;
}

bool pxwrDoc::Seek(s32 mode, s32 val)
{
	if (_b_file) {
		s32 seek_tbl[pxwrSEEK_num] = {SEEK_SET, SEEK_CUR, SEEK_END};
		if (fseek((FILE *)_p, val, seek_tbl[mode]))
			return false;
	} else {
		switch (mode) {
		case pxwrSEEK_set:
			if (val >= _len)
				return false;
			if (val < 0)
				return false;
			_ofs = val;
			break;
		case pxwrSEEK_cur:
			if (_ofs + val >= _len)
				return false;
			if (_ofs + val < 0)
				return false;
			_ofs += val;
			break;
		case pxwrSEEK_end:
			if (_len + val >= _len)
				return false;
			if (_len + val < 0)
				return false;
			_ofs = _len + val;
			break;
		}
	}

	return true;
}

void pxwrDoc::Close()
{
	if (_b_file && _p)
		fclose((FILE *)_p);

	_p = NULL;
	_b_file = false;
	_len = false;
	_ofs = 0;
}

bool pxwrDoc::w(const void *p, s32 size, s32 num)
{
	bool b_ret = false;

	if (!_p || !_b_file)
		goto End;

	if (fwrite(p, size, num, (FILE *)_p) != num)
		goto End;

	b_ret = true;
End:
	return b_ret;
}

bool pxwrDoc::w_arg(const char *fmt, ...)
{
	bool b_ret = false;
	char str[1024];
	s32 len;

	if (!_p || !_b_file)
		goto End;

	va_list ap;
	va_start(ap, fmt);
	vsprintf(str, fmt, ap);
	va_end(ap);

	len = strlen(str);
	if (fwrite(str, 1, len, (FILE *)_p) != len)
		goto End;

	b_ret = true;
End:
	return b_ret;
}

bool pxwrDoc::r(void *p, s32 size, s32 num)
{
	if (!_p)
		return false;

	bool b_ret = false;

	if (_b_file) {
		if (fread(p, size, num, (FILE *)_p) != num)
			goto End;
	} else {
		for (s32 i = 0; i < num; i++) {
			if (_ofs + size > _len)
				goto End;
			memcpy(&((s8 *)p)[i], &_p[_ofs], size);
			_ofs += size;
		}
	}

	b_ret = true;
End:
	return b_ret;
}

s32 pxwrDoc_v_chk(s32 val)
{
	u32 us;

	us = (u32)val;
	// 1byte(7bit)
	if (us < 0x80)
		return 1;
	// 2byte(14bit)
	if (us < 0x4000)
		return 2;
	// 3byte(21bit)
	if (us < 0x200000)
		return 3;
	// 4byte(28bit)
	if (us < 0x10000000)
		return 4;
	// 5byte(35bit)
	//	if( value < 0x800000000 ) return 5;
	if (us <= 0xffffffff)
		return 5;

	return 6;
}

// ..unsigned int
s32 pxwrDoc::v_w(s32 val, s32 *p_add)
{
	if (!_p)
		return 0;
	if (!_b_file)
		return 0;

	u8 a[5];
	u8 b[5];
	u32 us;

	us = (u32)val;

	a[0] = *((u8 *)(&us) + 0);
	a[1] = *((u8 *)(&us) + 1);
	a[2] = *((u8 *)(&us) + 2);
	a[3] = *((u8 *)(&us) + 3);
	a[4] = 0;

	// 1byte(7bit)
	if (us < 0x80) {
		if (fwrite(&a[0], 1, 1, (FILE *)_p) != 1)
			return false;
		if (p_add)
			*p_add += 1;
		return true;
	}

	// 2byte(14bit)
	if (us < 0x4000) {
		b[0] = ((a[0] << 0) & 0x7F) | 0x80;
		b[1] = (a[0] >> 7) | ((a[1] << 1) & 0x7F);
		if (fwrite(b, 1, 2, (FILE *)_p) != 2)
			return false;
		if (p_add)
			*p_add += 2;
		return true;
	}

	// 3byte(21bit)
	if (us < 0x200000) {
		b[0] = ((a[0] << 0) & 0x7F) | 0x80;
		b[1] = (a[0] >> 7) | ((a[1] << 1) & 0x7F) | 0x80;
		b[2] = (a[1] >> 6) | ((a[2] << 2) & 0x7F);
		if (fwrite(b, 1, 3, (FILE *)_p) != 3)
			return false;
		if (p_add)
			*p_add += 3;
		return true;
	}

	// 4byte(28bit)
	if (us < 0x10000000) {
		b[0] = ((a[0] << 0) & 0x7F) | 0x80;
		b[1] = (a[0] >> 7) | ((a[1] << 1) & 0x7F) | 0x80;
		b[2] = (a[1] >> 6) | ((a[2] << 2) & 0x7F) | 0x80;
		b[3] = (a[2] >> 5) | ((a[3] << 3) & 0x7F);
		if (fwrite(b, 1, 4, (FILE *)_p) != 4)
			return false;
		if (p_add)
			*p_add += 4;
		return true;
	}

	// 5byte(35bit)
	//	if( value < 0x800000000 ){
	if (us <= 0xffffffff) {

		b[0] = ((a[0] << 0) & 0x7F) | 0x80;
		b[1] = (a[0] >> 7) | ((a[1] << 1) & 0x7F) | 0x80;
		b[2] = (a[1] >> 6) | ((a[2] << 2) & 0x7F) | 0x80;
		b[3] = (a[2] >> 5) | ((a[3] << 3) & 0x7F) | 0x80;
		b[4] = (a[3] >> 4) | ((a[4] << 4) & 0x7F);
		if (fwrite(b, 1, 5, (FILE *)_p) != 5)
			return false;
		if (p_add)
			*p_add += 5;
		return true;
	}

	return false;
}

bool pxwrDoc::v_r(s32 *p)
{
	if (!_p)
		return false;

	s32 i;
	u8 a[5];
	u8 b[5];

	b[0] = b[1] = b[2] = b[3] = b[4] = 0;

	for (i = 0; i < 5; i++) {
		if (!pxwrDoc::r(&a[i], 1, 1))
			return false;
		if (!(a[i] & 0x80))
			break;
	}
	switch (i) {
	case 0:
		b[0] = (a[0] & 0x7F) >> 0;
		break;
	case 1:
		b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
		b[1] = (a[1] & 0x7F) >> 1;
		break;
	case 2:
		b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
		b[1] = ((a[1] & 0x7F) >> 1) | (a[2] << 6);
		b[2] = (a[2] & 0x7F) >> 2;
		break;
	case 3:
		b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
		b[1] = ((a[1] & 0x7F) >> 1) | (a[2] << 6);
		b[2] = ((a[2] & 0x7F) >> 2) | (a[3] << 5);
		b[3] = (a[3] & 0x7F) >> 3;
		break;
	case 4:
		b[0] = ((a[0] & 0x7F) >> 0) | (a[1] << 7);
		b[1] = ((a[1] & 0x7F) >> 1) | (a[2] << 6);
		b[2] = ((a[2] & 0x7F) >> 2) | (a[3] << 5);
		b[3] = ((a[3] & 0x7F) >> 3) | (a[4] << 4);
		b[4] = (a[4] & 0x7F) >> 4;
		break;
	case 5:
		return false;
	}

	*p = *((s32 *)b);

	return true;
}

bool pxwrDoc::r_txt(void *p_buf, s32 buf_size)
{
	if (!_p || !_b_file)
		return false;

	bool b_ret = false;
	u8 l = 0;
	s8 *p = (s8 *)p_buf;

	if (!pxwrDoc::r(&l, sizeof(l), 1))
		goto End;
	if (l >= buf_size)
		goto End;
	if (!pxwrDoc::r(p, 1, l))
		goto End;
	p[l] = '\0';

	b_ret = true;
End:

	return b_ret;
}

bool pxwrDoc::gets(s8 *p_buf, s32 buf_size)
{
	if (!_p || !_b_file)
		return false;

	if (!fgets(p_buf, buf_size, (FILE *)_p))
		return false;

	for (s32 i = strlen(p_buf) - 1; i >= 0; i--) {
		if (p_buf[i] == 0x0d || p_buf[i] == 0x0a)
			p_buf[i] = 0x00;
		else
			break;
	}
	return true;
}

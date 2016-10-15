#pragma once

#include <pxTypedef.h>
#include <vector>

#define PXWRDOC_MAXSIZE (10485760) // 10MB

class pxwrDoc
{
private:
	std::vector<char> data;
	s32 offset;

public:
	pxwrDoc();
	~pxwrDoc();

	bool Open_path(const char *path, const char *mode);
	bool Seek(s32 mode, s32 val);
	void Close();

	bool w(const void *p, s32 size, s32 num);
	bool r(void *p, s32 size, s32 num);

	s32 v_w(s32 val, s32 *p_add);
	bool v_r(s32 *p);

	s32 FileSize() const;
};

s32 pxwrDoc_v_chk(s32 val);

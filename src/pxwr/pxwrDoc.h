//
//  pxwrDoc.h
//
//  Created by Daisuke Amaya on 11/08/12.
//  Copyright 2011 Pixel. All rights reserved.
//

#ifndef pxwrDoc_H
#define pxwrDoc_H

#include <stdint.h>
#include <pxTypedef.h>

enum pxwrSEEK
{
	pxwrSEEK_set = 0,
	pxwrSEEK_cur,
	pxwrSEEK_end,
	pxwrSEEK_num
};

class pxwrDoc
{
private:
	
	s8   *_p    ;
	bool _b_file;
	s32  _len   ;
	s32  _ofs   ;
	
public:
	
	 pxwrDoc();
	~pxwrDoc();
	
	bool Open_path( const char *path, const s8* mode );
	bool SetRead  ( void *p, s32 len  );
	bool Seek     ( s32 mode, s32 val );
	void Close    ();
	
	bool w   ( const void *p, s32 size, s32 num );
	bool r   (       void *p, s32 size, s32 num );	
	void *GetFilePointer(){ return _p; }

	s32  v_w  ( s32 val, s32 *p_add );
	bool v_r  ( s32 *p  );

	bool r_txt( void *p    , s32 buf_size );
	bool gets ( s8   *p_buf, s32 buf_size );

	bool w_arg( const char *fmt, ... );

	s32 FileSize() const;
};

s32  pxwrDoc_v_chk( s32 val );

#endif

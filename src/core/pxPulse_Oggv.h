#ifndef pxPulse_Oggv_H
#define pxPulse_Oggv_H

#include "./pxTypedef.h"

#include <pxwrDoc.h>

#include "./pxPulse_PCM.h"

class pxPulse_Oggv
{
private:

	s32  _ch     ;
	s32  _sps2   ;
	s32  _smp_num;
	s32  _size   ;
	s8*  _p_data ;

	bool _SetInformation();

public :

	 pxPulse_Oggv();
	~pxPulse_Oggv();

	bool Load   ( const char* path   );
	bool Save   ( const char* path   ) const;
	bool Decode ( pxPulse_PCM *p_pcm ) const;
	void Release();
	bool GetInfo( s32* p_ch, s32* p_sps, s32* p_smp_num );
	s32  GetSize() const;
	bool Write  ( pxwrDoc *p_doc ) const;
	bool Read   ( pxwrDoc *p_doc );
	bool Copy   ( pxPulse_Oggv *p_dst ) const;
};

#endif

#ifndef pxPulse_Noise_H
#define pxPulse_Noise_H

#include <pxwrDoc.h>

#include "./pxTypedef.h"

#include "./pxPulse_Frequency.h"
#include "./pxPulse_Oscillator.h"
#include "./pxPulse_PCM.h"

enum pxWAVETYPE
{
	pxWAVETYPE_None = 0,
	pxWAVETYPE_Sine,
	pxWAVETYPE_Saw,
	pxWAVETYPE_Rect,
	pxWAVETYPE_Random,
	pxWAVETYPE_Saw2,
	pxWAVETYPE_Rect2,

	pxWAVETYPE_Tri    ,
	pxWAVETYPE_Random2,
	pxWAVETYPE_Rect3  ,
	pxWAVETYPE_Rect4  ,
	pxWAVETYPE_Rect8  ,
	pxWAVETYPE_Rect16 ,
	pxWAVETYPE_Saw3   ,
	pxWAVETYPE_Saw4   ,
	pxWAVETYPE_Saw6   ,
	pxWAVETYPE_Saw8   ,

	pxWAVETYPE_num,
};

typedef struct
{
	pxWAVETYPE type  ;
	f32        freq  ;
	f32        volume;
	f32        offset;
	bool       b_rev ;
}
pxNOISEDESIGN_OSCILLATOR;

typedef struct
{
	bool                     bEnable ;
	s32                      enve_num;
	sPOINT*                  enves   ;
	s32                      pan     ;
	pxNOISEDESIGN_OSCILLATOR main    ;
	pxNOISEDESIGN_OSCILLATOR freq    ;
	pxNOISEDESIGN_OSCILLATOR volu    ;
}
pxNOISEDESIGN_UNIT;


class pxPulse_Noise
{
private:

	s32                 _smp_num_44k;
	s32                 _unit_num   ;
	pxNOISEDESIGN_UNIT* _units      ;

public:
	 pxPulse_Noise();
	~pxPulse_Noise();

	bool Write       ( pxwrDoc* p_doc, s32* p_add        ) const;
	bool Read        ( pxwrDoc* p_doc, bool* pb_new_fmt  );
	bool Save        ( const char* path ) const;
	bool Load        ( const char* path, bool* p_bNew );
	void Release     ();
	bool Allocate    ( s32 unit_num, s32 envelope_num );
	bool Copy        ( pxPulse_Noise* p_dst       ) const;
	s32  Compare     ( const pxPulse_Noise* p_src ) const;
	void Fix();
	s32  SamplingSize( s32 ch, s32 sps, s32 bps ) const;

	void set_smp_num_44k( s32 num );

	s32  get_unit_num   () const;
	s32  get_smp_num_44k() const;
	f32  get_sec() const;
	pxNOISEDESIGN_UNIT* get_unit( s32 u );
};

#endif

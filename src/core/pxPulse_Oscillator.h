#ifndef pxPlus_Oscillator_H
#define pxPlus_Oscillator_H

#include "./pxTypedef.h"

typedef f64 (* FUNCTION_OSCILLATORGET )( s32 index );

class pxPulse_Oscillator
{
private:

	sPOINT *_p_point  ;
	s32    _point_num ;
	s32    _point_reso;
	s32    _volume    ;
	s32    _sample_num;

public:

	pxPulse_Oscillator();

	void ReadyGetSample( sPOINT *p_point, s32 point_num, s32 volume, s32 sample_num, s32 point_reso );
	f64  GetOneSample_Overtone ( s32 index );
	f64  GetOneSample_Coodinate( s32 index );
};

#endif

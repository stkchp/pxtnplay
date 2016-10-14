#ifndef pxPulse_Frequency_H
#define pxPulse_Frequency_H

#include "./pxTypedef.h"

class pxPulse_Frequency
{
private:

	f32 *_freq_table;
	f64 _GetDivideOctaveRate( s32 divi );

public:

	 pxPulse_Frequency();
	~pxPulse_Frequency();

	bool Init();

	f32        Get      ( s32 key     );
	f32        Get2     ( s32 key     );
	const f32* GetDirect( s32 *p_size );
};

#endif

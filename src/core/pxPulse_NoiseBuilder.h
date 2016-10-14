#ifndef pxPulse_NoiseBuilder_H
#define pxPulse_NoiseBuilder_H

#include "./pxTypedef.h"
#include "./pxPulse_Noise.h"

class pxPulse_NoiseBuilder
{
private:

	bool _b_init;
	s16* _p_tables[ pxWAVETYPE_num ];
	s32  _rand_buf [ 2 ];

	void _random_reset();
	s16  _random_get  ();

	pxPulse_Frequency *_freq;

public :

	 pxPulse_NoiseBuilder();
	~pxPulse_NoiseBuilder();

	bool Init();

	pxPulse_PCM *BuildNoise( pxPulse_Noise *p_noise, s32 ch, s32 sps, s32 bps ) const;
};

#endif

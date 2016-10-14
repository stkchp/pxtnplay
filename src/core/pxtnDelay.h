// '12/03/03

#ifndef pxtnDelay_H
#define pxtnDelay_H

#include <pxwrDoc.h>

#include "./pxtnMax.h"

enum DELAYUNIT
{
	DELAYUNIT_Beat = 0,
	DELAYUNIT_Meas,
	DELAYUNIT_Second,
	DELAYUNIT_num,
};

#define DEFAULT_DELAYSCALE       DELAYUNIT_Beat
#define DEFAULT_DELAYFREQUENCY    3.3f
#define DEFAULT_DELAYRATE        33.0f


class pxtnDelay
{
private:
	bool      _b_played;
	DELAYUNIT _unit    ;
	s32       _group   ;
	f32       _rate    ;
	f32       _freq    ;

	s32       _smp_num ;
	s32       _offset  ;
	s32       *_bufs[ MAX_CHANNEL ];
	s32       _rate_long;
	//f64 rate; // .rate / 100 (%)


public :

	 pxtnDelay();
	~pxtnDelay();

	bool Tone_Ready( s32 beat_num, f32 beat_tempo, s32 sps, s32 bps );
	void Tone_Supple( s32 ch, s32 *group_smps );
	void Tone_Increment();
	void Tone_Release  ();
	void Tone_Clear    ();

	bool Add_New    ( DELAYUNIT scale, f32 freq, f32 rate, s32 group );

	bool Write( pxwrDoc *p_doc ) const;
	bool Read ( pxwrDoc *p_doc, bool *pb_new_fmt );


	DELAYUNIT get_unit ()const;
	f32       get_freq ()const;
	f32       get_rate ()const;
	s32       get_group()const;

	void      Set( DELAYUNIT unit, f32 freq, f32 rate, s32 group );

	bool      get_played()const;
	void      set_played( bool b );
	bool      switch_played();
};



#endif

// '12/03/03

#ifndef pxtnUnit_H
#define pxtnUnit_H

#include "./pxTypedef.h"

#include <pxwrDoc.h>

#include "./pxtnMax.h"
#include "./pxtnWoice.h"


class pxtnUnit
{
private:

	bool _bOperated;
	bool _bPlayed;
	char _name[  MAX_TUNEUNITNAME + 1 ];

	//   TUNEUNITTONESTRUCT
	s32  _key_now;
	s32  _key_start;
	s32  _key_margin;
	s32  _portament_sample_pos;
	s32  _portament_sample_num;
	s32  _pan_vols     [ MAX_CHANNEL ];
	s32  _pan_times    [ MAX_CHANNEL ];
	s32  _pan_time_bufs[ MAX_CHANNEL ][ pxtnBUFSIZE_TIMEPAN ];
	s32  _v_VOLUME  ;
	s32  _v_VELOCITY;
	s32  _v_GROUPNO ;
	f32  _v_CORRECT ;

	const pxtnWoice *_p_w;

	pxtnVOICETONE _vts[ MAX_UNITCONTROLVOICE ];

public :
	 pxtnUnit();
	~pxtnUnit();

	void Tone_Init ();

	void Tone_Clear();

	void Tone_Reset_and_2prm( s32 voice_idx, s32 env_rls_clock, f32 offset_freq );
	void Tone_Envelope ();
	void Tone_KeyOn    ();
	void Tone_ZeroLives();
	void Tone_Key       ( s32 key );
	void Tone_Pan_Volume( s32 ch, s32 pan );
	void Tone_Pan_Time  ( s32 ch, s32 pan, s32 sps );

	void Tone_Velocity ( s32 val );
	void Tone_Volume   ( s32 val );
	void Tone_Portament( s32 val );
	void Tone_GroupNo  ( s32 val );
	void Tone_Correct  ( f32 val );

	void Tone_Sample   ( bool b_mute, s32 ch_num, s32 time_pan_index, s32 smooth_smp );
	void Tone_Supple   ( s32 *group_smps, s32 ch, s32 time_pan_index ) const;
	s32  Tone_Increment_Key   ();
	void Tone_Increment_Sample( f32 freq );

	void set_woice( const pxtnWoice *p_w );
	void set_name ( const char *name     );
	const pxtnWoice *get_woice() const;
	const char      *get_name () const;
	
	pxtnVOICETONE *get_tone( s32 voice_idx );

	void set_operated( bool b );
	void set_played  ( bool b );
	bool get_operated() const;
	bool get_played  () const;

	
//	bool Write   ( pxwrDoc *p_doc, s32 idx ) const;
//	bool Read    ( pxwrDoc *p_doc, bool *pb_new_fmt );
	bool Read_v3x( pxwrDoc *p_doc, bool *pb_new_fmt, s32 *p_group );
	bool Read_v1x( pxwrDoc *p_doc,                   s32 *p_group );

};

#endif

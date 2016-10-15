// '12/03/13

// support: 11025,22050,44100Hz / 8bit,16bit / mono, stereo.

#ifndef pxtoneVomit_H
#define pxtoneVomit_H

#include <pxTypedef.h>
#include <pxwrDoc.h>

class pxtoneVomit
{
private:

	bool _b_init ;
	bool _b_vomit;

	void *_pxtn  ;

public :

	 pxtoneVomit();
	~pxtoneVomit();

	bool Init ();
	bool Read ( pxwrDoc *p_doc );
	bool Clear();
	bool Start( s32 sampling_position, f32 fade_in_sec );

	bool set_quality ( s32 ch, s32 sps, s32 bps );
	bool set_loop    ( bool b_loop       );
	bool set_volume  ( f32 volume        ); // 1.0f = 100%
	s32  set_fade    ( s32 fade, f32 sec );

	bool is_vomiting() const;

	const char *get_title     () const;
	const char *get_comment   () const;
	const char *get_last_error() const;

	bool get_info( s32 *p_beat_num, f32 *p_beat_tempo, s32 *p_beat_clock, s32 *p_meas_num ) const;
	s32  get_meas_repeat() const;
	s32  get_meas_play  () const;

	bool vomit( void *p_buf, s32 buf_size );
};


s32 pxtoneVomit_Calc_sample_num( s32 meas_num, s32 beat_num, s32 sps, f32 beat_tempo );

#endif

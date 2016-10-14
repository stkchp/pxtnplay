// '12/03/03

#ifndef pxtnMaster_H
#define pxtnMaster_H

#include "./pxTypedef.h"

#include <pxwrDoc.h>

class pxtnMaster
{
private:
	s32 _beat_num   ;
	f32 _beat_tempo ;
	s32 _beat_clock ;
	s32 _meas_num   ;
	s32 _repeat_meas;
	s32 _last_meas  ;
	s32 _volume_    ;

public :
	 pxtnMaster();
	~pxtnMaster();

	void Reset();

	void Set( s32    beat_num, f32    beat_tempo, s32    beat_clock );
	void Get( s32 *p_beat_num, f32 *p_beat_tempo, s32 *p_beat_clock ) const;

	s32  get_beat_num   ()const;
	f32  get_beat_tempo ()const;
	s32  get_beat_clock ()const;
	s32  get_meas_num   ()const;
	s32  get_repeat_meas()const;
	s32  get_last_meas  ()const;
	s32  get_last_clock ()const;
	s32  get_play_meas  ()const;

	void set_meas_num   ( s32 meas_num );
	void set_repeat_meas( s32 meas     );
	void set_last_meas  ( s32 meas     );

	void AdjustMeasNum  ( s32 clock    );

	s32  get_this_clock( s32 meas, s32 beat, s32 clock ) const;

	bool io_w_v5          ( pxwrDoc *p_doc, s32 rough  ) const;
	bool io_r_v5          ( pxwrDoc *p_doc, bool *pb_new_fmt );
	s32  io_r_v5_EventNum ( pxwrDoc *p_doc );

	bool io_r_x4x         ( pxwrDoc *p_doc, bool *pb_new_fmt );
	s32  io_r_x4x_EventNum( pxwrDoc *p_doc );
};

#endif

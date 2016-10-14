#ifndef pxtnService_H
#define pxtnService_H


#include "./pxTypedef.h"

#include <pxwrDoc.h>
#include "./pxPulse_NoiseBuilder.h"

#include "./pxtnMax.h"
#include "./pxtnText.h"
#include "./pxtnDelay.h"
#include "./pxtnOverDrive.h"
#include "./pxtnMaster.h"
#include "./pxtnWoice.h"
#include "./pxtnUnit.h"
#include "./pxtnEvelist.h"

#define PXTONEERRORSIZE 64

enum pxtnERRORS
{
	pxtnERR_None = 0,
	pxtnERR_NoiseBuilder_Init,
	pxtnERR_EventList_Allocate,
	pxtnERR_Allocate,

	pxtnERR_io_w_Version,
	pxtnERR_io_w_ExeVersion,
	pxtnERR_io_w_Dummy,
	pxtnERR_io_w_TagCode,
	pxtnERR_io_w_Master,
	pxtnERR_io_w_Events,

	pxtnERR_io_w_Name,
	pxtnERR_io_w_Comment,
	pxtnERR_io_w_Delay,
	pxtnERR_io_w_OverDrive,
	pxtnERR_io_w_PCM,
	pxtnERR_io_w_PTV,
	pxtnERR_io_w_PTN,
	pxtnERR_io_w_OGGV,

	pxtnERR_io_w_WoiceAssist,
	pxtnERR_io_w_UnitNum    ,
	pxtnERR_io_w_UnitAssist ,
	pxtnERR_io_w_EndSize    ,

	pxtnERR_io_r_TagCode    ,
	pxtnERR_io_r_AntiEdit   ,
	pxtnERR_io_r_UnitNum    ,
	pxtnERR_io_r_Master     ,
	pxtnERR_io_r_Events     ,
	pxtnERR_io_r_PCM        ,
	pxtnERR_io_r_PTV        ,
	pxtnERR_io_r_PTN        ,
	pxtnERR_io_r_OGGV       ,
	pxtnERR_io_r_Delay      ,
	pxtnERR_io_r_OverDrive  ,
	pxtnERR_io_r_Title      ,
	pxtnERR_io_r_Coment     ,
	pxtnERR_io_r_WoiceAssist,
	pxtnERR_io_r_UnitAssist ,

	pxtnERR_io_r_v4_Master  ,
	pxtnERR_io_r_v4_Event   ,
	pxtnERR_io_r_v3_Unit    ,
	pxtnERR_io_r_v1_Project ,
	pxtnERR_io_r_v1_Unit    ,
	pxtnERR_io_r_v1_PCM     ,
	pxtnERR_io_r_v1_Event   ,
	pxtnERR_io_r_Unknown    ,

	pxtnERR_io_r_Version    ,
	pxtnERR_io_r_VerUnknown ,
	pxtnERR_io_r_VersionExe ,
	pxtnERR_io_r_Dummy      ,

	pxtnERR_io_r_FactSize   ,
	pxtnERR_io_r_Seek       ,

	pxtnERR_io_r_BeatClock  ,

	pxtnERR_num
};

typedef struct
{
	s32 meas_start  ;
	s32 meas_end    ;
	s32 meas_repeat ;
	s32 start_sample; // for restart.
	s32 fadein_msec ;
//	f32 volume      ; // 0.0 - 1.0
}
pxtnVOMITPREPARATION;


class pxtnService
{
private:


	enum _enum_FMTVER
	{
		_enum_FMTVER_x1x = 0, // fix event num = 10000
		_enum_FMTVER_x2x,     // no version of exe
		_enum_FMTVER_x3x,     // unit has voice / basic-key for only view
		_enum_FMTVER_x4x,     // unit has event
		_enum_FMTVER_v5 ,
	};

	pxtnERRORS _err;

	s32 _ch, _sps, _bps;

	pxPulse_NoiseBuilder* _ptn_bldr;

	s32 _delay_max;	s32 _delay_num;	pxtnDelay     **_delays;
	s32 _ovdrv_max;	s32 _ovdrv_num;	pxtnOverDrive **_ovdrvs;
	s32 _woice_max;	s32 _woice_num;	pxtnWoice     **_woices;
	s32 _unit_max ;	s32 _unit_num ;	pxtnUnit      **_units ;

	s32 _group_num;

	bool _ReadVersion      ( pxwrDoc* p_doc, _enum_FMTVER* p_fmt_ver, u16 *p_exe_ver );
	bool _ReadTuneItems    ( pxwrDoc* p_doc );
	bool _x1x_Project_Read ( pxwrDoc* p_doc );

	bool _io_Read_Delay    ( pxwrDoc* p_doc, bool* pb_new_fmt );
	bool _io_Read_OverDrive( pxwrDoc* p_doc, bool* pb_new_fmt );
	bool _io_Read_Woice    ( pxwrDoc* p_doc, bool* pb_new_fmt, pxtnWOICETYPE type );
	bool _io_Read_OldUnit  ( pxwrDoc* p_doc, bool* pb_new_fmt, s32 ver            );

	bool _io_assiWOIC_w    ( pxwrDoc* p_doc, s32 idx          ) const;
	bool _io_assiWOIC_r    ( pxwrDoc* p_doc, bool* pb_new_fmt );
	bool _io_assiUNIT_w    ( pxwrDoc* p_doc, s32 idx          ) const;
	bool _io_assiUNIT_r    ( pxwrDoc* p_doc, bool* pb_new_fmt );

	bool _io_UNIT_num_w    ( pxwrDoc* p_doc ) const;
	s32  _io_UNIT_num_r    ( pxwrDoc* p_doc, bool *pb_new_fmt );

	bool _x3x_CorrectKeyEvent();
	bool _x3x_AddCorrectEvent();
	void _x3x_SetVoiceNames  ();

	//////////////
	// Vomit..
	//////////////
	bool _vmt_b_ready   ;
	bool _vmt_b_init    ;

	bool _vmt_b_mute    ;
	bool _vmt_b_loop    ;

	s32  _vmt_smp_smooth;
	f32  _vmt_clock_rate; // as the sample
	s32  _vmt_smp_count ;
	s32  _vmt_smp_start ;
	s32  _vmt_smp_end   ;
	s32  _vmt_smp_repeat;
		
	s32  _vmt_fade_count;
	s32  _vmt_fade_max  ;
	s32  _vmt_fade_fade ;
	f32  _vmt_master_vol;
		
	s32  _vmt_top       ;
	s32  _vmt_smp_skip  ;
	s32  _vmt_time_pan_index;

	f32  _vmt_bt_tempo  ;

	// for make now-meas
	s32  _vmt_bt_clock  ;
	s32  _vmt_bt_num    ;

	s32* _vmt_group_smps;


	const EVERECORD*   _vmt_p_eve;

	pxPulse_Frequency* _vmt_freq ;

	void _vmt_constructor();
	void _vmt_destructer ();
	bool _vmt_init       ();


	void _vmt_ResetVoiceOn( pxtnUnit* p_u, s32 w ) const;
	void _vmt_InitUnitTone();
	bool _vmt_PXTONE_SAMPLE( void* p_data );

public :

	 pxtnService();
	~pxtnService();

	pxtnText*    text  ;
	pxtnMaster*  master;
	pxtnEvelist* evels ;

	bool Init         ( bool b_reserve_evels ); // ptc reserves evels.
	void Clear        ( bool b_release_evels ); // ptc doesn't release evels.

	bool Save         ( pxwrDoc* p_doc, bool bTune, u16 exe_ver );
	s32  PreCountEvent( pxwrDoc* p_doc, s32 max_unit            );
	bool Read         ( pxwrDoc* p_doc, bool bPermitPTTUNE      );
	void AdjustMeasNum();

//	const char* GetIOError();

	const char* get_last_error_text() const;

	bool Tone_Ready();
	void Tone_Clear();

	s32  Group_Num() const;

	// delay.
	s32  Delay_Num() const;
	s32  Delay_Max() const;
	bool Delay_Set         ( s32 idx, DELAYUNIT unit, f32 freq, f32 rate, s32 group );
	bool Delay_Add         (          DELAYUNIT unit, f32 freq, f32 rate, s32 group );
	bool Delay_Remove      ( s32 idx );
	bool Delay_ReadyTone   ( s32 idx );
	pxtnDelay* Delay_Get   ( s32 idx );


	// over drive.
	s32  OverDrive_Num() const;
	s32  OverDrive_Max() const;
	bool OverDrive_Set          ( s32 idx, f32 cut, f32 amp, s32 group );
	bool OverDrive_Add          (          f32 cut, f32 amp, s32 group );
	bool OverDrive_Remove       ( s32 idx );
	void OverDrive_ReadyTone    ( s32 idx );
	pxtnOverDrive* OverDrive_Get( s32 idx );

	// woice.
	s32  Woice_Num() const;
	s32  Woice_Max() const;
	const pxtnWoice* Woice_Get( s32 idx ) const;
	pxtnWoice*       Woice_Get_variable( s32 idx );

	bool Woice_Load     ( s32 idx, const char *path, pxtnWOICETYPE type, bool *pb_new_fmt );
	bool Woice_ReadyTone( s32 idx );
	void Woice_Remove   ( s32 idx );
	void Woice_Replace  ( s32 old_place, s32 new_place );

	// unit.
	s32   Unit_Num() const;
	s32   Unit_Max() const;
	const pxtnUnit* Unit_Get         ( s32 idx ) const;
	pxtnUnit*       Unit_Get_variable( s32 idx );

	void Unit_Remove   ( s32 idx );
	void Unit_Replace  ( s32 old_place, s32 new_place );
	bool Unit_AddNew   ();
	void Unit_SetOpratedAll( bool b );
	void Unit_Solo( s32 idx );

	// q
	void Quality_Set( s32    ch, s32    sps, s32    bps );
	void Quality_Get( s32 *p_ch, s32 *p_sps, s32 *p_bps ) const;


	//////////////
	// Vomit..
	//////////////

	bool vmt_is_ready() const;

	void vmt_set_mute( bool b );
	void vmt_set_loop( bool b );
	void vmt_set_fade( s32 fade, s32 msec );
	void vmt_set_master_volume( f32 v );

	s32  vmt_get_now_clock      () const;
	s32  vmt_get_sampling_offset() const;

	bool vmt_preparation( const pxtnVOMITPREPARATION *p_build );

	bool Vomit( void* p_buf, s32 size );
};

s32 pxtnService_vmt_CalcSampleNum( s32 meas_num, s32 beat_num, s32 sps, f32 beat_tempo );


#endif

// '12/03/03

#include <stdlib.h>
#include <string.h>

#include "./pxtnMaster.h"
#include "./pxtnEvelist.h"

pxtnMaster::pxtnMaster()
{
	Reset();
}

pxtnMaster::~pxtnMaster()
{
}

void pxtnMaster::Reset()
{
	_beat_num    = EVENTDEFAULT_BEATNUM  ;
	_beat_tempo  = EVENTDEFAULT_BEATTEMPO;
	_beat_clock  = EVENTDEFAULT_BEATCLOCK;
	_meas_num    = 1;
	_repeat_meas = 0;
	_last_meas   = 0;
}

void  pxtnMaster::Set( s32    beat_num, f32    beat_tempo, s32    beat_clock )
{
	_beat_num   = beat_num;
	_beat_tempo = beat_tempo;
	_beat_clock = beat_clock;
}

void  pxtnMaster::Get( s32 *p_beat_num, f32 *p_beat_tempo, s32 *p_beat_clock ) const
{
	if( p_beat_num   ) *p_beat_num   = _beat_num  ;
	if( p_beat_tempo ) *p_beat_tempo = _beat_tempo;
	if( p_beat_clock ) *p_beat_clock = _beat_clock;
}

s32 pxtnMaster::get_beat_num   ()const{ return _beat_num   ;}
f32 pxtnMaster::get_beat_tempo ()const{ return _beat_tempo ;}
s32 pxtnMaster::get_beat_clock ()const{ return _beat_clock ;}
s32 pxtnMaster::get_meas_num   ()const{ return _meas_num   ;}
s32 pxtnMaster::get_repeat_meas()const{ return _repeat_meas;}
s32 pxtnMaster::get_last_meas  ()const{ return _last_meas  ;}

s32 pxtnMaster::get_last_clock ()const
{
	return _last_meas * _beat_clock * _beat_num;
}

s32 pxtnMaster::get_play_meas  ()const
{
	if( _last_meas ) return _last_meas;
	return _meas_num;
}

s32 pxtnMaster::get_this_clock( s32 meas, s32 beat, s32 clock ) const
{
	return _beat_num * _beat_clock * meas + _beat_clock * beat + clock;
}

//void  pxtnMaster::set_meas_num   ( s32 meas_num ){ _meas_num    = meas_num; }


void pxtnMaster::AdjustMeasNum( s32 clock )
{
	s32 m_num;
	s32 b_num;

	b_num   = ( clock + _beat_clock  - 1 ) / _beat_clock;
	m_num   = ( b_num + _beat_num    - 1 ) / _beat_num;
	if( _meas_num    <= m_num       ) _meas_num  = m_num;
	if( _repeat_meas >= _meas_num   ) _repeat_meas = 0;
	if( _last_meas   >  _meas_num   ) _last_meas = _meas_num;
}

void pxtnMaster::set_meas_num( s32 meas_num )
{
	if( meas_num < 1                ) meas_num = 1;
	if( meas_num <= _repeat_meas    ) meas_num = _repeat_meas + 1;
	if( meas_num <  _last_meas      ) meas_num = _last_meas;
	_meas_num = meas_num;
}



void  pxtnMaster::set_repeat_meas( s32 meas ){ if( meas < 0 ) meas = 0; _repeat_meas = meas; }
void  pxtnMaster::set_last_meas  ( s32 meas ){ if( meas < 0 ) meas = 0; _last_meas   = meas; }























bool pxtnMaster::io_w_v5( pxwrDoc *p_doc, s32 rough ) const
{
/*	s16 beat_clock  ;
	s8  beat_num    ;
	f32 beat_tempo  ;

	{
		s32 bn, bc;
		TuneData_Master_Get( &bn, &beat_tempo, &bc );
		beat_num   = (s8 )_beat_num bn;
		beat_clock = (s16)( bc / rough );
	}
*/
	u32 size   =          15;
	s16 bclock = _beat_clock / rough;
	s32 clock_repeat = bclock * _beat_num * get_repeat_meas();// TuneData_Master_GetRepeatMeas();
	s32 clock_last   = bclock * _beat_num * get_last_meas  ();// TuneData_Master_GetLastMeas(  );
	s8  bnum   = _beat_num  ;
	f32 btempo = _beat_tempo;
	if( !p_doc->w( &size, sizeof(u32), 1 ) ) return false;
	if( !p_doc->w( &bclock      , sizeof(s16), 1 ) ) return false;
	if( !p_doc->w( &bnum        , sizeof(s8 ), 1 ) ) return false;
	if( !p_doc->w( &btempo      , sizeof(f32), 1 ) ) return false;
	if( !p_doc->w( &clock_repeat, sizeof(s32 ), 1 ) ) return false;
	if( !p_doc->w( &clock_last  , sizeof(s32 ), 1 ) ) return false;

	return true;
}

bool pxtnMaster::io_r_v5( pxwrDoc *p_doc, bool *pb_new_fmt )
{
	s16 beat_clock  ;
	s8  beat_num    ;
	f32 beat_tempo  ;
	s32 clock_repeat;
	s32 clock_last  ;

	u32 size;

	if( !p_doc->r( &size, sizeof(s32),        1 ) ) return false;
	if( size != 15 ){ *pb_new_fmt = true; return false; }

	if( !p_doc->r( &beat_clock  ,sizeof(s16), 1 ) ) return false;
	if( !p_doc->r( &beat_num    ,sizeof(s8 ), 1 ) ) return false;
	if( !p_doc->r( &beat_tempo  ,sizeof(f32), 1 ) ) return false;
	if( !p_doc->r( &clock_repeat,sizeof(s32), 1 ) ) return false;
	if( !p_doc->r( &clock_last  ,sizeof(s32), 1 ) ) return false;

	_beat_clock = beat_clock;
	_beat_num   = beat_num  ;
	_beat_tempo = beat_tempo;

	set_repeat_meas( clock_repeat / ( beat_num * beat_clock ) );
	set_last_meas  ( clock_last   / ( beat_num * beat_clock ) );

//	TuneData_Master_Set( beat_num, beat_tempo, beat_clock );
//	TuneData_Master_SetRepeatMeas( clock_repeat / ( beat_num * beat_clock ) );
//	TuneData_Master_SetLastMeas(   clock_last   / ( beat_num * beat_clock ) );

	return true;
}


s32 pxtnMaster::io_r_v5_EventNum( pxwrDoc *p_doc )
{
	u32 size;
	if( !p_doc->r( &size, sizeof(s32),  1 ) ) return 0;
	if( size != 15                          ) return 0;
	s8 buf[ 15 ];
	if( !p_doc->r(  buf , sizeof(s8), 15  ) ) return 0;
	return 5;
}

/////////////////////////////////
// file io
/////////////////////////////////

// マスター情報(8byte) ================
typedef struct
{
	u16 data_num;        // １イベントのデータ数。現在は 3 ( clock / status / volume ）
	u16 rrr;
	u32 event_num;
}
_x4x_MASTER;
/*
bool pxtnMaster::io_w_x4x( pxwrDoc *p_doc, s32 rough )
{
	_x4x_MASTER mast;
	s32         size;
	s32         tempo_binary;
	f32         beat_tempo;
	s32         beat_clock;
	s32         beat_num;
	s32         repeat_clock;
	s32         last_clock;

//	if( !fp ) return false;

//	TuneData_Master_Get( &beat_num, &beat_tempo, &beat_clock );
	memcpy( &tempo_binary, &_beat_tempo, sizeof(s32) );
	repeat_clock   = beat_clock * beat_num * TuneData_Master_GetRepeatMeas();
	last_clock     = beat_clock * beat_num * TuneData_Master_GetLastMeas();
	mast.data_num  = 3; // status / clock / volume
	mast.rrr       = 0;
	mast.event_num = 3; // beat_clock / beat_tempo / beat_num / (repeat_clock)

	size = 0;

	size += pxwrDoc_v_chk( EVENTKIND_BEATCLOCK );
	size += pxwrDoc_v_chk( 0 );
	size += pxwrDoc_v_chk( beat_clock / rough    );

	size += pxwrDoc_v_chk( EVENTKIND_BEATTEMPO );
	size += pxwrDoc_v_chk( 0 );
	size += pxwrDoc_v_chk( tempo_binary          );

	size += pxwrDoc_v_chk( EVENTKIND_BEATNUM   );
	size += pxwrDoc_v_chk( 0 );
	size += pxwrDoc_v_chk( beat_num              );

	if( repeat_clock )
	{
		mast.event_num++;
		size += pxwrDoc_v_chk( EVENTKIND_REPEAT    );
		size += pxwrDoc_v_chk( repeat_clock / rough  );
		size += pxwrDoc_v_chk( 0                     );
	}

	if( last_clock )
	{
		mast.event_num++;
		size += pxwrDoc_v_chk( EVENTKIND_LAST      );
		size += pxwrDoc_v_chk( last_clock   / rough  );
		size += pxwrDoc_v_chk( 0                     );
	}

	// 書き出し
	size += sizeof( _x4x_MASTER );
	if( !p_doc->w( &size, sizeof(u32), 1 ) ) return false;
	if( !p_doc->w( &mast, sizeof( _x4x_MASTER ), 1 ) ) return false;

	if( !p_doc->v_w( EVENTKIND_BEATCLOCK, NULL ) ) return false;
	if( !p_doc->v_w( 0,                   NULL ) ) return false;
	if( !p_doc->v_w( beat_clock / rough,  NULL ) ) return false;

	if( !p_doc->v_w( EVENTKIND_BEATTEMPO, NULL ) ) return false;
	if( !p_doc->v_w( 0,                   NULL ) ) return false;
	if( !p_doc->v_w( tempo_binary,        NULL ) ) return false;
											  
	if( !p_doc->v_w( EVENTKIND_BEATNUM,   NULL ) ) return false;
	if( !p_doc->v_w( 0,                   NULL ) ) return false;
	if( !p_doc->v_w( beat_num,            NULL ) ) return false;

	if( repeat_clock )
	{
		if( !p_doc->v_w( EVENTKIND_REPEAT,     NULL ) ) return false;
		if( !p_doc->v_w( repeat_clock / rough, NULL ) ) return false;
		if( !p_doc->v_w( 0,                    NULL ) ) return false;
	}

	if( last_clock )
	{
		if( repeat_clock > 0 ) last_clock -= repeat_clock; 
		if( !p_doc->v_w( EVENTKIND_LAST,       NULL ) ) return false;
		if( !p_doc->v_w( last_clock   / rough, NULL ) ) return false;
		if( !p_doc->v_w( 0,                    NULL ) ) return false;
	}

	return true;
}
*/

//----------------------



// 読み込み(プロジェクト)
bool pxtnMaster::io_r_x4x( pxwrDoc *p_doc, bool *pb_new_fmt )
{
	_x4x_MASTER mast;
	s32         size;
	s32         e;
	s32         status;
	s32         clock;
	s32         volume;
	s32         absolute;

	s32         beat_clock, beat_num, repeat_clock, last_clock;
	f32         beat_tempo;

	memset( &mast, 0, sizeof( _x4x_MASTER ) );
	if( !p_doc->r( &size,                     4, 1 ) ) return false;
	if( !p_doc->r( &mast, sizeof( _x4x_MASTER ), 1 ) ) return false;

	// 未対応
	if( mast.data_num != 3 ){ *pb_new_fmt = true; return false; }
	if( mast.rrr           ){ *pb_new_fmt = true; return false; }

	beat_clock   = EVENTDEFAULT_BEATCLOCK;
	beat_num     = EVENTDEFAULT_BEATNUM;
	beat_tempo   = EVENTDEFAULT_BEATTEMPO;
	repeat_clock = 0;
	last_clock   = 0;

	absolute = 0;
	for( e = 0; e < (s32)mast.event_num; e++ )
	{
		if( !p_doc->v_r( &status ) ) break;
		if( !p_doc->v_r( &clock  ) ) break;
		if( !p_doc->v_r( &volume ) ) break;
		absolute += clock;
		clock     = absolute;

		switch( status )
		{
		case EVENTKIND_BEATCLOCK: beat_clock   =  volume;                        if( clock  ) return false; break;
		case EVENTKIND_BEATTEMPO: memcpy( &beat_tempo, &volume, sizeof(f32) ); if( clock  ) return false; break;
		case EVENTKIND_BEATNUM  : beat_num     =  volume;                        if( clock  ) return false; break;
		case EVENTKIND_REPEAT   : repeat_clock =  clock ;                        if( volume ) return false; break;
		case EVENTKIND_LAST     : last_clock   =  clock ;                        if( volume ) return false; break;
		default: *pb_new_fmt = true; return false; // 未対応
		}
	}

	if( e != mast.event_num ) return false;

	_beat_num   = beat_num  ;
	_beat_tempo = beat_tempo;
	_beat_clock = beat_clock;

	set_repeat_meas( repeat_clock / ( beat_num * beat_clock ) );
	set_last_meas  ( last_clock   / ( beat_num * beat_clock ) );

//	TuneData_Master_Set( beat_num, beat_tempo, beat_clock );
//	TuneData_Master_SetRepeatMeas( repeat_clock / ( beat_num * beat_clock ) );
//	TuneData_Master_SetLastMeas(   last_clock   / ( beat_num * beat_clock ) );

	return true;
}

// イベントの数を取得するのみ
s32 pxtnMaster::io_r_x4x_EventNum( pxwrDoc *p_doc )
{
	_x4x_MASTER mast;
	s32         size;
	s32         work;
	s32         e;

	memset( &mast, 0, sizeof( _x4x_MASTER ) );
	if( !p_doc->r( &size,                     4, 1 ) ) return 0;
	if( !p_doc->r( &mast, sizeof( _x4x_MASTER ), 1 ) ) return 0;

	if( mast.data_num != 3 ) return 0;

	for( e = 0; e < (s32)mast.event_num; e++ )
	{
		if( !p_doc->v_r( &work ) ) return 0;
		if( !p_doc->v_r( &work ) ) return 0;
		if( !p_doc->v_r( &work ) ) return 0;
	}

	return mast.event_num;
}

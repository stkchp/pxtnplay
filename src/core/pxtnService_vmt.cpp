#include <stdlib.h>
#include <string.h>


#include "./pxtnService.h"

static bool _malloc_zero( void **pp, s32 size )
{
	*pp = malloc( size );
	if(  !( *pp ) ) return false;
	memset( *pp, 0, size );
	return true;
}

static void _free_null( void **pp )
{
	if( *pp ){ free( *pp ); *pp = NULL; }
}

// ------------------
// critical section
// ------------------
/*
void pxtnService::_vmt_CS_Start()
{
	InitializeCriticalSection( &_vmt_CS );
	_vmt_b_CS = true;
}

bool pxtnService::_vmt_CS_Enter( void )
{
	if( !_vmt_b_CS ) return false;
	EnterCriticalSection( &_vmt_CS );
	return true;
}

void pxtnService::_vmt_CS_Leave( void )
{
	LeaveCriticalSection( &_vmt_CS );
}

void pxtnService::_vmt_CS_End()
{
	_vmt_b_CS = false;
	DeleteCriticalSection( &_vmt_CS );
}
*/



void pxtnService::_vmt_constructor()
{
	_vmt_b_init       = false;

//	_vmt_b_CS        = false;

	_vmt_b_ready      = false;
			   
	_vmt_b_mute       = true ;
	_vmt_b_loop       = true ;
	
	_vmt_fade_fade    =     0;
	_vmt_master_vol   =     1;
	_vmt_bt_clock     =     0;
	_vmt_bt_num       =     0;

	_vmt_freq         = NULL ;
	_vmt_group_smps   = NULL ;
	_vmt_p_eve        = NULL ;

	_vmt_master_vol   =  1.0f;



//	_vmt_CS_Start();

}



void pxtnService::_vmt_destructer()
{
//	vmt_Release_withCS();
//	_vmt_CS_End();

	SAFE_DELETE( _vmt_freq );
	if( _vmt_group_smps ) free( _vmt_group_smps ); _vmt_group_smps = NULL;
}

bool pxtnService::_vmt_init()
{
	_vmt_freq = new pxPulse_Frequency();
	if( !_vmt_freq->Init() ) return false;
	if( !_malloc_zero( (void **)&_vmt_group_smps, sizeof(s32) * _group_num ) ) return false;
	_vmt_b_init = true;
	return true;
}


////////////////////////////////////////////////
// ユニット ////////////////////////////////////
////////////////////////////////////////////////

void pxtnService::_vmt_ResetVoiceOn( pxtnUnit *p_u, s32 w ) const
{
	const pxtnVOICEWORK *p_work ;
	const pxtnVOICEUNIT *p_voice;

	const pxtnWoice *p_w = Woice_Get( w );

	if( !p_w ) return;

	p_u->set_woice( p_w );

	for( s32 v = 0; v < p_w->get_voice_num(); v++ )
	{
		p_work  = p_w->get_work ( v );
		p_voice = p_w->get_voice( v );

		f32 ofs_freq = 0;
		if( p_voice->voice_flags & PTV_VOICEFLAG_BEATFIT )
		{
			ofs_freq = ( p_work->smp_body_w * _vmt_bt_tempo ) / ( 44100 * 60 * p_voice->correct );
		}
		else
		{
			ofs_freq = _vmt_freq->Get( EVENTDEFAULT_BASICKEY - p_voice->basic_key ) * p_voice->correct;
		}
		p_u->Tone_Reset_and_2prm( v, (s32)( p_work->env_release / _vmt_clock_rate ), ofs_freq );
	}
}


void pxtnService::_vmt_InitUnitTone( void )
{
	for( s32 u = 0; u < _unit_num; u++ )
	{
		pxtnUnit *p_u = Unit_Get_variable( u );
		p_u->Tone_Init();
		_vmt_ResetVoiceOn( p_u, EVENTDEFAULT_VOICENO );
	}
}


bool pxtnService::_vmt_PXTONE_SAMPLE( void *p_data )
{
	// envelope..
	for( s32 u = 0; u < _unit_num;  u++ ) _units[ u ]->Tone_Envelope();

	s32 clock = (s32)( _vmt_smp_count / _vmt_clock_rate );

	// events..
	for( ; _vmt_p_eve && _vmt_p_eve->clock <= clock; _vmt_p_eve = _vmt_p_eve->next )
	{
		s32           u    = _vmt_p_eve->unit_no;
		pxtnUnit      *p_u = _units[ u ];
		pxtnVOICETONE *p_tone;
		const pxtnWoice *p_w;
		const pxtnVOICEWORK *p_work;

		switch( _vmt_p_eve->kind )
		{
		case EVENTKIND_ON       : 
			{

				s32 on_count = (s32)( (_vmt_p_eve->clock + _vmt_p_eve->value - clock) * _vmt_clock_rate );
				if( on_count <= 0 ){ p_u->Tone_ZeroLives(); break; }

				p_u->Tone_KeyOn();

				if( !( p_w = p_u->get_woice() ) ) break;
				for( s32 v = 0; v < p_w->get_voice_num(); v++ )
				{
					p_tone = p_u->get_tone( v );
					p_work = p_w->get_work( v );

					// release..
					if( p_work->env_release )
					{
						s32        max_life_count1 = (s32)( ( _vmt_p_eve->value - ( clock - _vmt_p_eve->clock ) ) * _vmt_clock_rate ) + p_work->env_release;
						s32        max_life_count2;
						s32        c    = _vmt_p_eve->clock + _vmt_p_eve->value + p_tone->env_release_clock;
						EVERECORD* next = NULL;
						for( EVERECORD* p = _vmt_p_eve->next; p; p = p->next )
						{
							if( p->clock > c ) break;
							if( p->unit_no == u && p->kind == EVENTKIND_ON ){ next = p; break; }
						}
						if( !next ) max_life_count2 = _vmt_smp_end - (s32)( clock   * _vmt_clock_rate );
						else        max_life_count2 = (s32)( ( next->clock -      clock ) * _vmt_clock_rate );
						if( max_life_count1 < max_life_count2 ) p_tone->life_count = max_life_count1;
						else                                    p_tone->life_count = max_life_count2;
					}
					// no-release..
					else
					{
						p_tone->life_count = (s32)( ( _vmt_p_eve->value - ( clock - _vmt_p_eve->clock ) ) * _vmt_clock_rate );
					}

					if( p_tone->life_count > 0 )
					{
						p_tone->on_count  = on_count;
						p_tone->smp_pos   = 0;
						p_tone->env_pos   = 0;
						if( p_work->env_size ) p_tone->env_volume = p_tone->env_start  =   0; // envelope
						else                   p_tone->env_volume = p_tone->env_start  = 128; // no-envelope
					}
				}
				break;
			}

		case EVENTKIND_KEY       : p_u->Tone_Key( _vmt_p_eve->value ); break;
		case EVENTKIND_PAN_VOLUME: p_u->Tone_Pan_Volume( _ch, _vmt_p_eve->value ); break;
		case EVENTKIND_PAN_TIME  : p_u->Tone_Pan_Time  ( _ch, _vmt_p_eve->value, _sps ); break;
		case EVENTKIND_VELOCITY  : p_u->Tone_Velocity  ( _vmt_p_eve->value ); break;
		case EVENTKIND_VOLUME    : p_u->Tone_Volume    ( _vmt_p_eve->value ); break;
		case EVENTKIND_PORTAMENT : p_u->Tone_Portament ( (s32)( _vmt_p_eve->value * _vmt_clock_rate ) ); break;
		case EVENTKIND_BEATCLOCK : break;
		case EVENTKIND_BEATTEMPO : break;
		case EVENTKIND_BEATNUM   : break;
		case EVENTKIND_REPEAT    : break;
		case EVENTKIND_LAST      : break;
		case EVENTKIND_VOICENO   : _vmt_ResetVoiceOn( p_u, _vmt_p_eve->value ); break;
		case EVENTKIND_GROUPNO   : p_u->Tone_GroupNo  (              _vmt_p_eve->value    ); break;
		case EVENTKIND_CORRECT   : p_u->Tone_Correct  ( *( (f32*)(&_vmt_p_eve->value) ) ); break;
		}
	}

	// sampling..
	for( s32 u = 0; u < _unit_num; u++ ) _units[ u ]->Tone_Sample( _vmt_b_mute, _ch, _vmt_time_pan_index, _vmt_smp_smooth );

	for( s32 ch = 0; ch < _ch; ch++ )
	{
		for( s32 g = 0; g < _group_num; g++ ) _vmt_group_smps[ g ] = 0;
		for( s32 u = 0; u < _unit_num ; u++ ) _units[ u ]->Tone_Supple( _vmt_group_smps, ch, _vmt_time_pan_index );
		for( s32 o = 0; o < _ovdrv_num; o++ ) _ovdrvs[ o ]->Tone_Supple( _vmt_group_smps );
		for( s32 d = 0; d < _delay_num; d++ ) _delays[ d ]->Tone_Supple( ch, _vmt_group_smps );

		// collect.
		s32 work = 0;
		for( s32 g = 0; g < _group_num; g++ ) work += _vmt_group_smps[ g ];

		// fade..
		if( _vmt_fade_fade ) work = work * ( _vmt_fade_count >> 8 ) / _vmt_fade_max;

		// master volume
		work = (s32)( work * _vmt_master_vol );

		// to buffer..
		if( _bps == 8 ) work = work >> 8;
		if( work >  _vmt_top ) work =  _vmt_top;
		if( work < -_vmt_top ) work = -_vmt_top;
		if( _bps == 8 ) *( (u8  *)p_data + ch ) = (u8 )( work + 128 );
		else            *( (s16 *)p_data + ch ) = (s16)( work       );
	}

	// --------------
	// increments..

	_vmt_smp_count++;
	_vmt_time_pan_index = ( _vmt_time_pan_index + 1 ) & ( pxtnBUFSIZE_TIMEPAN - 1 );

	for( s32 u = 0; u < _unit_num;  u++ )
	{
		s32 key_now = _units[ u ]->Tone_Increment_Key();
		_units[ u ]->Tone_Increment_Sample( _vmt_freq->Get2( key_now ) *_vmt_smp_skip );
	}

	// delay
	for( s32 d = 0; d < _delay_num; d++ ) _delays[ d ]->Tone_Increment();

	// fade out
	if( _vmt_fade_fade < 0 )
	{
		if( _vmt_fade_count > 0  ) _vmt_fade_count--;
		else return false;
	}
	// fade in
	else if( _vmt_fade_fade > 0 )
	{
		if( _vmt_fade_count < (_vmt_fade_max << 8) ) _vmt_fade_count++;
		else                                         _vmt_fade_fade = 0;
	}

	if( _vmt_smp_count >= _vmt_smp_end )
	{
		if( !_vmt_b_loop ) return false;
		_vmt_smp_count = _vmt_smp_repeat;
		s32 kousokuka_dekirukamo;
		_vmt_p_eve = evels->get_Records();
		_vmt_InitUnitTone();
	}

	return true;
}


///////////////////////
// get / set 
///////////////////////

bool pxtnService::vmt_is_ready( void ) const
{
	return _vmt_b_ready;
}

s32 pxtnService::vmt_get_now_clock( void ) const
{
	if( _vmt_clock_rate ) return (s32)( _vmt_smp_count / _vmt_clock_rate );
	return 0;
}

void pxtnService::vmt_set_mute( bool b ){ _vmt_b_mute = b; }
void pxtnService::vmt_set_loop( bool b ){ _vmt_b_loop = b; }
void pxtnService::vmt_set_fade( s32 fade, s32 msec )
{
	_vmt_fade_max = ( _sps * msec / 1000 ) >> 8;
	if(      fade < 0 ){ _vmt_fade_fade  = -1; _vmt_fade_count = _vmt_fade_max << 8; } // out
	else if( fade > 0 ){ _vmt_fade_fade  =  1; _vmt_fade_count =  0;                 } // in
	else               { _vmt_fade_fade =   0;                                       } // off
}


////////////////////////////
// preparation
////////////////////////////

/*
void pxtnService::vmt_Release_withCS( void )
{
	if( _vmt_CS_Enter() )
	{
		_vmt_b_ready = false;

//		_free_null( (void**)&_units          );
//		_free_null( (void**)&_delays         );
//		_free_null( (void**)&_ovdrvs         );
//		_free_null( (void**)&_p_group_sample );

		_vmt_CS_Leave();
	}
}
*/

// preparation
bool pxtnService::vmt_preparation( const pxtnVOMITPREPARATION *p_tune/*, pxtnService *pxtn*/ )
{
	bool b_ret = false;

	if( !_vmt_b_init ) return false;

	_vmt_b_ready = false;

//	if( !_vmt_CS_Enter() ) return false;

	if( p_tune->meas_start  >= p_tune->meas_end ) goto End;
	if( p_tune->meas_repeat >= p_tune->meas_end ) goto End;

//	_vmt_master_vol     = p_tune->volume;

	_vmt_bt_clock       = master->get_beat_clock();
	_vmt_bt_num         = master->get_beat_num  ();
	_vmt_bt_tempo       = master->get_beat_tempo();

	_vmt_clock_rate     = (f32)( 60.0f * (f64)_sps / ( (f64)_vmt_bt_tempo * (f64)_vmt_bt_clock ) );

	_vmt_smp_skip       = ( 44100 / _sps );

	if( _bps == 8 ) _vmt_top =   127;
	else            _vmt_top = 32767;

	_vmt_time_pan_index = 0;

	_vmt_smp_start  = (s32)( (f64)p_tune->meas_start  * (f64)_vmt_bt_num * (f64)_vmt_bt_clock * _vmt_clock_rate );
	_vmt_smp_end    = (s32)( (f64)p_tune->meas_end    * (f64)_vmt_bt_num * (f64)_vmt_bt_clock * _vmt_clock_rate );
	_vmt_smp_repeat = (s32)( (f64)p_tune->meas_repeat * (f64)_vmt_bt_num * (f64)_vmt_bt_clock * _vmt_clock_rate );
	if( p_tune->start_sample && p_tune->start_sample < _vmt_smp_end ) _vmt_smp_start = p_tune->start_sample;
	_vmt_smp_count  = _vmt_smp_start;
	_vmt_smp_smooth = _sps / 250; // (0.004sec) // (0.010sec)


	if( p_tune->fadein_msec > 0 ) vmt_set_fade( 1, p_tune->fadein_msec );
	else                          vmt_set_fade( 0, 0 );

	Tone_Clear();

	_vmt_p_eve = evels->get_Records();

	_vmt_InitUnitTone();

	_vmt_b_ready = true;
	b_ret        = true;
End:

//	_vmt_CS_Leave();

//	if( !b_ret ) vmt_Release_withCS();

	return b_ret;
}


s32 pxtnService::vmt_get_sampling_offset( void ) const
{
	return _vmt_smp_count;
}


void pxtnService::vmt_set_master_volume( f32 v )
{
	if( v < 0 ) v = 0;
	if( v > 1 ) v = 1;
	_vmt_master_vol = v;
}

////////////////////
// 
////////////////////

bool pxtnService::Vomit( void* p_buf, s32 size )
{
	s32  t = 0;
	s32  offset;
	s32  block_size;
	u8   bytes[ 4 ];
	u8   *p;
	bool b_last = false;

//	if( !_vmt_CS_Enter() ) return false;

	if( !_vmt_b_ready ){ b_last = true; goto End; }

	block_size = _ch * _bps / 8;

	if( _bps == 8 )
	{
		p = (u8 *)p_buf;
		for( s32 b = 0; b < size; b++ )
		{
			offset = ( t % block_size );
			if( !offset && !_vmt_PXTONE_SAMPLE( bytes ) ) b_last = true;
			p[ b ] = bytes[ offset ];
			t++;
		}
	}
	else
	{
		p = (u8 *)p_buf;
		for( s32 b = 0; b < size; b++ )
		{
			offset = ( t % block_size );
			if( !offset && !_vmt_PXTONE_SAMPLE( bytes ) ) b_last = true;
			p[ b ] = bytes[ offset ];
			t++;
		}
	}

End:
//	_vmt_CS_Leave();

	return b_last ? false : true;
}



s32 pxtnService_vmt_CalcSampleNum( s32 meas_num, s32 beat_num, s32 sps, f32 beat_tempo )
{
	u32 total_beat_num;
	u32 sample_num    ;

	if( !beat_tempo ) return 0;

	total_beat_num = meas_num * beat_num;
	sample_num     = (u32)( (f64)sps * 60 * (f64)total_beat_num / (f64)beat_tempo );

	return sample_num;
}

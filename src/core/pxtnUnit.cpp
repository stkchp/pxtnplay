// '12/03/03

#include <string.h>

#include "./pxtnUnit.h"
#include "./pxtnEvelist.h"

pxtnUnit::pxtnUnit()
{
	_bPlayed = true;
	strcpy( _name, "no name" );
}

pxtnUnit::~pxtnUnit()
{
}

void pxtnUnit::Tone_Init()
{
	_v_GROUPNO      = EVENTDEFAULT_GROUPNO ;
	_v_VELOCITY     = EVENTDEFAULT_VELOCITY;
	_v_VOLUME       = EVENTDEFAULT_VOLUME  ;
	_v_CORRECT      = EVENTDEFAULT_CORRECT ;
	_portament_sample_num = 0;
	_portament_sample_pos = 0;

	for( s32 i = 0; i < MAX_CHANNEL; i++ )
	{
		_pan_vols [ i ] = 64;
		_pan_times[ i ] =  0;
	}
}

void pxtnUnit::Tone_Clear()
{
	for( s32 i = 0; i < MAX_CHANNEL; i++ ) memset( _pan_time_bufs[ i ], 0, sizeof(s32) * pxtnBUFSIZE_TIMEPAN );
}

void pxtnUnit::Tone_Reset_and_2prm( s32 voice_idx, s32 env_rls_clock, f32 offset_freq )
{
	pxtnVOICETONE* p_tone = &_vts[ voice_idx ];
	p_tone->life_count    = 0;
	p_tone->on_count      = 0;
	p_tone->smp_pos       = 0;
	p_tone->smooth_volume = 0;
	p_tone->env_release_clock = env_rls_clock;
	p_tone->offset_freq       = offset_freq  ;
}

void pxtnUnit::set_woice( const pxtnWoice *p_w )
{
	_p_w        = p_w;
	_key_now    = EVENTDEFAULT_KEY;
	_key_margin = 0;
	_key_start  = EVENTDEFAULT_KEY;
}

void pxtnUnit::set_name ( const char *name )
{
	u32 max_ = MAX_TUNEUNITNAME;

	if( strlen( name ) > max_ )
	{
		memcpy( _name, name, max_ );
		_name[ max_ ] = '\0';
		u8 c = _name[ max_ -1 ];
		bool b_zenkaku = false;
		if( c >= 0x81 && c <= 0x9F ) b_zenkaku = true;
		if( c >= 0xE0 && c <= 0xEF ) b_zenkaku = true;
		if( b_zenkaku ) _name[ max_ -1 ] = '\0';
	}
	else
	{
		strcpy( _name, name );
	}
}

const char *pxtnUnit::get_name () const{ return _name; }

void pxtnUnit::set_operated( bool b )
{
	_bOperated = b;
}

void pxtnUnit::set_played  ( bool b )
{
	_bPlayed   = b;
}

bool pxtnUnit::get_operated() const{ return _bOperated; }
bool pxtnUnit::get_played  () const{ return _bPlayed  ; }

void pxtnUnit::Tone_ZeroLives()
{
	for( s32 i = 0; i < MAX_CHANNEL; i++ ) _vts[ i ].life_count = 0;
}

void pxtnUnit::Tone_KeyOn()
{
	_key_now    = _key_start + _key_margin;
	_key_start  = _key_now;
	_key_margin = 0;
}

void pxtnUnit::Tone_Key( s32 key )
{
	_key_start            = _key_now;
	_key_margin           = key - _key_start;
	_portament_sample_pos = 0;
}

void pxtnUnit::Tone_Pan_Volume( s32 ch, s32 pan )
{
	_pan_vols[ 0 ] = 64;
	_pan_vols[ 1 ] = 64;
	if( ch == 2 )
	{
		if( pan >= 64 )_pan_vols[ 0 ] = 128 - pan;
		else           _pan_vols[ 1 ] =       pan;
	}
}

void pxtnUnit::Tone_Pan_Time( s32 ch, s32 pan, s32 sps )
{
	_pan_times[ 0 ] = 0;
	_pan_times[ 1 ] = 0;

	if( ch == 2 )
	{
		if( pan >= 64 )
		{
			_pan_times[ 0 ]  = pan - 64; if( _pan_times[ 0 ] > 63 ) _pan_times[ 0 ] = 63;
			_pan_times[ 0 ] /= 44100 / sps;
		}
		else
		{
			_pan_times[ 1 ]  = 64 - pan; if( _pan_times[ 1 ] > 63 ) _pan_times[ 1 ] = 63;
			_pan_times[ 1 ] /= 44100 / sps;
		}
	}
}

void pxtnUnit::Tone_Velocity ( s32  val ){ _v_VELOCITY           = val; }
void pxtnUnit::Tone_Volume   ( s32  val ){ _v_VOLUME             = val; }
void pxtnUnit::Tone_Portament( s32  val ){ _portament_sample_num = val; }
void pxtnUnit::Tone_GroupNo  ( s32  val ){ _v_GROUPNO            = val; }
void pxtnUnit::Tone_Correct  ( f32 val ){ _v_CORRECT            = val; }

void pxtnUnit::Tone_Envelope()
{
	if( !_p_w ) return;

	for( s32 v = 0; v < _p_w->get_voice_num(); v++ )
	{
		const pxtnVOICEWORK *p_work = _p_w->get_work( v );
		pxtnVOICETONE       *p_tone = &_vts         [ v ];
		if( p_tone->life_count > 0 && p_work->env_size )
		{
			if( p_tone->on_count > 0 )
			{
				if( p_tone->env_pos < p_work->env_size )
				{
					p_tone->env_volume = p_work->p_env[ p_tone->env_pos ];
					p_tone->env_pos++;
				}
			}
			// release.
			else
			{
				p_tone->env_volume = p_tone->env_start + ( 0 - p_tone->env_start ) * p_tone->env_pos / p_work->env_release;
				p_tone->env_pos++;
			}
		}
	}
}

void pxtnUnit::Tone_Sample( bool b_mute, s32 ch_num, s32 time_pan_index, s32 smooth_smp )
{
	if( !_p_w               ) return;
	if( b_mute && !_bPlayed )
	{
		for( s32 ch = 0; ch < ch_num; ch++ ) _pan_time_bufs[ ch ][ time_pan_index ] = 0;
		return;
	}

	for( s32 ch = 0; ch < MAX_CHANNEL; ch++ )
	{
		s32 time_pan_buf = 0;

		for( s32 v = 0; v < _p_w->get_voice_num(); v++ )
		{
			pxtnVOICETONE       *p_vt = &_vts         [ v ];
			const pxtnVOICEWORK *p_vw = _p_w->get_work( v );

			s32 work = 0;

			if( p_vt->life_count > 0 )
			{
				s32 pos = (s32)p_vt->smp_pos * 4 + ch * 2;
				work += *( (s16*)&p_vw->p_smp_w[ pos ] );

				if( ch_num == 1 )
				{
					work += *( (s16*)&p_vw->p_smp_w[ pos + 2 ] );
					work = work / 2;
				}

				work = ( work * _v_VELOCITY )   / 128;
				work = ( work * _v_VOLUME   )   / 128;
				work =   work * _pan_vols[ ch ] /  64;

				if( p_vw->env_size ) work = work * p_vt->env_volume / 128;

				// smooth tail
				if( _p_w->get_voice( v )->voice_flags & PTV_VOICEFLAG_SMOOTH && p_vt->life_count < smooth_smp )
				{
					work = work * p_vt->life_count / smooth_smp;
				}
			}
			time_pan_buf += work;
		}
		_pan_time_bufs[ ch ][ time_pan_index ] = time_pan_buf;
	}
}

void pxtnUnit::Tone_Supple( s32 *group_smps, s32 ch, s32 time_pan_index ) const
{
	s32 idx = ( time_pan_index - _pan_times[ ch ] ) & ( pxtnBUFSIZE_TIMEPAN - 1 );
	group_smps[ _v_GROUPNO ] += _pan_time_bufs[ ch ][ idx ];
}

s32 pxtnUnit::Tone_Increment_Key()
{
	// prtament..
	if( _portament_sample_num && _key_margin )
	{
		if( _portament_sample_pos < _portament_sample_num )
		{
			_portament_sample_pos++;
			_key_now = (s32)( _key_start + (f64)_key_margin * _portament_sample_pos / _portament_sample_num );
		}
		else
		{
			_key_now    = _key_start + _key_margin;
			_key_start  = _key_now;
			_key_margin = 0;
		}
	}
	else
	{
		_key_now = _key_start + _key_margin;
	}
	return _key_now;
}

void pxtnUnit::Tone_Increment_Sample( f32 freq )
{
	if( !_p_w ) return;

	for( s32 v = 0; v < _p_w->get_voice_num(); v++ )
	{
		const pxtnVOICEWORK *p_vw = _p_w->get_work( v );
		pxtnVOICETONE       *p_vt = &_vts         [ v ];

		if( p_vt->life_count > 0 ) p_vt->life_count--;
		if( p_vt->life_count > 0 )
		{
			p_vt->on_count--;

			p_vt->smp_pos += p_vt->offset_freq * _v_CORRECT * freq;

			if( p_vt->smp_pos >= p_vw->smp_body_w )
			{
				if( _p_w->get_voice( v )->voice_flags & PTV_VOICEFLAG_WAVELOOP )
				{
					if( p_vt->smp_pos >= p_vw->smp_body_w ) p_vt->smp_pos -= p_vw->smp_body_w;
					if( p_vt->smp_pos >= p_vw->smp_body_w ) p_vt->smp_pos  = 0;
				}
				else
				{
					p_vt->life_count = 0;
				}
			}

			// OFF
			if( p_vt->on_count == 0 && p_vw->env_size )
			{
				p_vt->env_start = p_vt->env_volume;
				p_vt->env_pos   = 0;
			}
		}
	}
}


const pxtnWoice *pxtnUnit::get_woice() const{ return _p_w; }

pxtnVOICETONE *pxtnUnit::get_tone( s32 voice_idx )
{
	return &_vts[ voice_idx ];
}


// v1x (20byte) ================= 
typedef struct
{
	char name[ MAX_TUNEUNITNAME ];
	u16  type ;
	u16  group;
}
_x1x_UNIT;

bool pxtnUnit::Read_v1x( pxwrDoc *p_doc, s32 *p_group )
{
	_x1x_UNIT unit;
	s32       size;

	if( !p_doc->r( &size, 4,                   1 ) ) return false;
	if( !p_doc->r( &unit, sizeof( _x1x_UNIT ), 1 ) ) return false;
	if( (pxtnWOICETYPE)unit.type != pxtnWOICE_PCM  ) return false;

	memcpy( _name, unit.name, MAX_TUNEUNITNAME ); _name[ MAX_TUNEUNITNAME ] = '\0';
	*p_group = unit.group;

	return true;
}

///////////////////
// pxtnUNIT x3x
///////////////////

typedef struct
{
	u16 type ;
	u16 group;
}
_x3x_UNIT;

bool pxtnUnit::Read_v3x( pxwrDoc *p_doc, bool *pb_new_fmt, s32 *p_group )
{
	_x3x_UNIT unit;
	s32       size;

	if( !p_doc->r( &size, 4,                   1 ) ) return false;
	if( !p_doc->r( &unit, sizeof( _x3x_UNIT ), 1 ) ) return false;
	if( (pxtnWOICETYPE)unit.type != pxtnWOICE_PCM &&
		(pxtnWOICETYPE)unit.type != pxtnWOICE_PTV &&
		(pxtnWOICETYPE)unit.type != pxtnWOICE_PTN )
	{
		*pb_new_fmt = true;
		return false;
	}
	*p_group = unit.group;

	return true;
}

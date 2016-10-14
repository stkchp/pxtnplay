#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pxwrDoc.h>

#include "./pxPulse_PCM.h"


typedef struct
{
	u16 formatID;      // PCM:0x0001
	u16 ch;            // 
	u32 sps;           // 
	u32 byte_per_sec;  // byte per sec.
	u16 block_size;    // 
	u16 bps;           // bit per sample.
	u16 ext;           // no use for pcm.
}
WAVEFORMATCHUNK;

static bool _malloc_zero( void **pp, s32 size )
{
	*pp = malloc( size ); if( !( *pp ) ) return false;
	memset( *pp, 0, size );              return true;
}

static void _free_null( void **pp )
{
	if( *pp ){ free( *pp ); *pp = NULL; }
}

void pxPulse_PCM::Release()
{
	if( _p_smp ) free( _p_smp ); _p_smp = NULL;
	_ch       =    0;
	_sps      =    0;
	_bps      =    0;
	_smp_head =    0;
	_smp_body =    0;
	_smp_tail =    0;
}

pxPulse_PCM::pxPulse_PCM()
{
	_p_smp    = NULL;
	Release();
}

pxPulse_PCM::~pxPulse_PCM()
{
	Release();
}

void *pxPulse_PCM::Devolve_SamplingBuffer()
{
	void *p = _p_smp;
	_p_smp = NULL;
	return p;
}


bool pxPulse_PCM::Make( s32 ch, s32 sps, s32 bps, s32 sample_num )
{
	Release();

	s32 size;
	_p_smp    = NULL;
	_ch       = ch ;
	_sps      = sps;
	_bps      = bps;
	_smp_head = 0;
	_smp_body = sample_num;
	_smp_tail = 0;

	// bit / sample is 8 or 16
	if( _bps != 8 && _bps != 16 ) return false;
	size = _smp_body * _bps * _ch / 8;

	if( !( _p_smp = (u8 *)malloc( size ) ) ) return false;

	if( _bps == 8 ) memset( _p_smp, 128, size );
	else            memset( _p_smp,   0, size );

	return true;
}



bool pxPulse_PCM::Load( const char *path )
{
	pxwrDoc doc;
	bool    b_ret = false;
	s8      buf[16];
	u32     size;
	WAVEFORMATCHUNK format;

	_p_smp = NULL;

	if( !doc.Open_path( path, "rb" ) ) goto End;

	// 'RIFFxxxxWAVEfmt '
	if( !doc.r( buf, sizeof(s8), 16 ) ) goto End;

	if( buf[ 0] != 'R' || buf[ 1] != 'I' || buf[ 2] != 'F' || buf[ 3] != 'F' ||
		buf[ 8] != 'W' || buf[ 9] != 'A' || buf[10] != 'V' || buf[11] != 'E' ||
		buf[12] != 'f' || buf[13] != 'm' || buf[14] != 't' || buf[15] != ' ' )
	{
		goto End;
	}

	// read format.
	if( !doc.r( &size  , sizeof(u32),    1 ) ) goto End;
	if( !doc.r( &format,          18,    1 ) ) goto End;
	
	if( format.formatID != 0x0001            ) goto End;// for only PCM
	if( format.ch  != 1 && format.ch  !=  2  ) goto End;// ch : 1 or 2
	if( format.bps != 8 && format.bps != 16  ) goto End;// bit / sample : 8 or 16


	// find 'data'
	if( !doc.Seek( SEEK_SET, 12 )            ) goto End; // skip 'RIFFxxxxWAVE'

	while( 1 )
	{
		if( !doc.r( buf  , sizeof(s8),  4 ) ) goto End;
		if( !doc.r( &size, sizeof(u32), 1 ) ) goto End;
		if( buf[0] == 'd' && buf[1] == 'a' && buf[2] == 't' && buf[3] == 'a' ) break;
		if( !doc.Seek( SEEK_CUR, size )     ) goto End;
	}
/*
	_ch  = format.ch;
	_sps = format.sps;
	_bps = format.bps;
	_smp_head = 0;
	_smp_body = size * 8 / format.bps / format.ch;
	_smp_tail = 0;
*/
	if( !Make( format.ch, format.sps, format.bps, size * 8 / format.bps / format.ch ) ) goto End;

//	if( !( _p_smp = (u8 *)malloc( size ) ) ) goto End;
	if( !doc.r( _p_smp, sizeof(u8), size ) ) goto End;

	b_ret = true;

End:

	if( !b_ret && _p_smp ){ free( _p_smp ); _p_smp = NULL; }
	return b_ret;
}

bool pxPulse_PCM::Save( const char *path, const char *pstrLIST ) const
{
	if( !_p_smp ) return false;

	WAVEFORMATCHUNK format;
	pxwrDoc         doc;
	bool            b_ret = false;
	u32             riff_size;
	u32             fact_size; // num sample.
	u32             list_size; // num list text.
	u32             isft_size;
	u32             sample_size;

	bool bText;

	s8 tag_RIFF[4] = {'R','I','F','F'};
	s8 tag_WAVE[4] = {'W','A','V','E'};
	s8 tag_fmt_[8] = {'f','m','t',' ', 0x12,0,0,0};
	s8 tag_fact[8] = {'f','a','c','t', 0x04,0,0,0};
	s8 tag_data[4] = {'d','a','t','a'};

	s8 tag_LIST[4] = {'L','I','S','T'};
	s8 tag_INFO[8] = {'I','N','F','O','I','S','F','T'};


	if( pstrLIST && strlen( pstrLIST ) ) bText = true ;
	else                                 bText = false;

	sample_size         = ( _smp_head + _smp_body + _smp_tail ) * _ch * _bps / 8;

	format.formatID     = 0x0001;// PCM
	format.ch           = (u16) _ch;
	format.sps          = (u32) _sps;
	format.bps          = (u16) _bps;
	format.byte_per_sec = (u32)(_sps * _bps * _ch / 8);
	format.block_size   = (u16)(       _bps * _ch / 8);
	format.ext          = 0;

	fact_size = ( _smp_head + _smp_body + _smp_tail );
	riff_size  = sample_size;
	riff_size +=  4;// 'WAVE'
	riff_size += 26;// 'fmt '
	riff_size += 12;// 'fact'
	riff_size +=  8;// 'data'

	if( bText )
	{
		isft_size  = strlen( pstrLIST );
		list_size  = 4 + 4 + 4 + isft_size; // "INFO" + "ISFT" + size + ver_Text;
		riff_size +=         8 + list_size; // 'LIST'
	}
	else
	{
		isft_size = 0;
		list_size = 0;
	}

	// open file..
	if( !doc.Open_path( path, "wb" ) ) goto End;

	if( !doc.w( tag_RIFF,   sizeof(s8),  4 ) ) goto End;
	if( !doc.w( &riff_size, sizeof(u32), 1 ) ) goto End;
	if( !doc.w( tag_WAVE,   sizeof(s8),  4 ) ) goto End;
	if( !doc.w( tag_fmt_,   sizeof(s8),  8 ) ) goto End;
	if( !doc.w( &format,    18,          1 ) ) goto End;
		
	if( bText )
	{
		if( !doc.w( tag_LIST,   sizeof(s8),          4 ) ) goto End;
		if( !doc.w( &list_size, sizeof(u32),         1 ) ) goto End;
		if( !doc.w( tag_INFO,   sizeof(s8),          8 ) ) goto End;
		if( !doc.w( &isft_size, sizeof(u32),         1 ) ) goto End;
		if( !doc.w( pstrLIST,   sizeof(s8),  isft_size ) ) goto End;
	}
															   
	if( !doc.w( tag_fact,     sizeof(s8),     8 ) ) goto End;
	if( !doc.w( &fact_size,   sizeof(u32),    1 ) ) goto End;
	if( !doc.w( tag_data,     sizeof(s8),     4 ) ) goto End;
	if( !doc.w( &sample_size, sizeof(s32),    1 ) ) goto End;
	if( !doc.w( _p_smp, sizeof(s8), sample_size ) ) goto End;

	b_ret = true;

End:

	return b_ret;
}

// stereo / mono 
bool pxPulse_PCM::_Convert_ChannelNum( s32 new_ch )
{
	u8  *p_work = NULL;
	s32  sample_size;
	s32  work_size;
	s32  a,b;
	s32  temp1;
	s32  temp2;

	sample_size = ( _smp_head + _smp_body + _smp_tail ) * _ch * _bps / 8;

	if( _p_smp == NULL   ) return false;
	if( _ch    == new_ch ) return true;


	// mono to stereo --------
	if( new_ch == 2 )
	{
		work_size = sample_size * 2;
		p_work    = (u8 *)malloc( work_size );
		if( !p_work ) return false;

		switch( _bps )
		{
		case  8:
			b = 0;
			for( a = 0; a < sample_size; a++ )
			{
				p_work[b  ] = _p_smp[a];
				p_work[b+1] = _p_smp[a];
				b += 2;
			}
			break;
		case 16:
			b = 0;
			for( a = 0; a < sample_size; a += 2 )
			{
				p_work[b  ] = _p_smp[a];
				p_work[b+1] = _p_smp[a+1];
				p_work[b+2] = _p_smp[a];
				p_work[b+3] = _p_smp[a+1];
				b += 4;
			}
			break;
		}

	}
	// stereo to mono --------
	else
	{
		work_size = sample_size / 2;
		p_work    = (u8 *)malloc( work_size );
		if( !p_work ) return false;

		switch( _bps )
		{
		case  8:
			b = 0;
			for( a = 0; a < sample_size; a+= 2 )
			{
				temp1     = (s32)_p_smp[a] + (s32)_p_smp[a+1];
				p_work[b] = (u8)( temp1 / 2 );
				b++;
			}
			break;
		case 16:
			b = 0;
			for( a = 0; a < sample_size; a += 4 )
			{
				temp1                = *((s16 *)(&_p_smp[a  ]));
				temp2                = *((s16 *)(&_p_smp[a+2]));
				*(s16 *)(&p_work[b]) =  (s16)( ( temp1 + temp2 ) / 2 );
				b += 2;
			}
			break;
		}
	}

	// release once.
	free( _p_smp );
	_p_smp = NULL;

	if( !( _p_smp = (u8*)malloc( work_size ) ) ){ free( p_work ); return false; }
	memcpy( _p_smp, p_work, work_size );
	free( p_work );

	// update param.
	_ch = new_ch;

	return true;
}

// change bps
bool pxPulse_PCM::_Convert_BitPerSample( s32 new_bps )
{
	u8  *p_work;
	s32 sample_size;
	s32 work_size;
	s32 a,b;
        
	s32 temp1;

	if( !_p_smp         ) return false;
	if( _bps == new_bps ) return true ;

	sample_size = ( _smp_head + _smp_body + _smp_tail ) * _ch * _bps / 8;

	switch( new_bps )
	{
	// 16 to 8 --------
	case  8:
		work_size = sample_size / 2;
		p_work    = (u8 *)malloc( work_size );
		if( !p_work ) return false;
		b = 0;
		for( a = 0; a < sample_size; a += 2 )
		{
			temp1     = *((s16*)(&_p_smp[a]));
			temp1     =  (temp1/0x100) + 128;
			p_work[b] =  (u8)temp1;
			b++;
		}
		break;
	//  8 to 16 --------
	case 16:
		work_size = sample_size * 2;
		p_work    = (u8 *)malloc( work_size );
		if( !p_work ) return false;
		b = 0;
		for( a = 0; a < sample_size; a++ )
		{
			temp1 = _p_smp[a];
			temp1 = ( temp1 - 128 ) * 0x100;
			*((s16 *)(&p_work[b])) = (s16)temp1;
			b += 2;
		}
		break;

	default: return false;
	}

	// release once.
	free( _p_smp );
	_p_smp = NULL;

	if( !( _p_smp = (u8*)malloc( work_size ) ) ){ free( p_work ); return false; }
	memcpy( _p_smp, p_work, work_size );
	free( p_work );

	// update param.
	_bps = new_bps;

	return true;
}

// sps
bool pxPulse_PCM::_Convert_SamplePerSecond( s32 new_sps )
{
	bool  b_ret = false;
	s32   sample_num;
	s32   work_size;

	s32   head_size, body_size, tail_size;

	u8   *p1byte_data;
	u16  *p2byte_data;
	u32  *p4byte_data;

	u8   *p1byte_work = NULL;
	u16  *p2byte_work = NULL;
	u32  *p4byte_work = NULL;


	s32   a, b;

	if( !_p_smp         ) return false;
	if( _sps == new_sps ) return true ;

//	s32 old_smp = _smp_body;

	head_size = _smp_head * _ch * _bps / 8;
	body_size = _smp_body * _ch * _bps / 8;
	tail_size = _smp_tail * _ch * _bps / 8;

	head_size = (s32)( ( (f64)head_size * (f64)new_sps + (f64)(_sps) - 1 ) / _sps );
	body_size = (s32)( ( (f64)body_size * (f64)new_sps + (f64)(_sps) - 1 ) / _sps );
	tail_size = (s32)( ( (f64)tail_size * (f64)new_sps + (f64)(_sps) - 1 ) / _sps );

	work_size = head_size + body_size + tail_size;

	// stereo 16bit ========
	if(     _ch == 2 && _bps == 16 )
	{
		_smp_head = head_size  / 4;
		_smp_body = body_size  / 4;
		_smp_tail = tail_size  / 4;
		sample_num      = work_size  / 4;
		work_size       = sample_num * 4;
		p4byte_data     = (u32 *)_p_smp;
		if( !_malloc_zero( (void **)&p4byte_work, work_size ) ) goto End;
		for( a = 0; a < sample_num; a++ )
		{
			b = (s32)( (f64)a * (f64)(_sps) / (f64)new_sps );
			p4byte_work[a] = p4byte_data[b];
		}

	}
	// mono 8bit ========
	else if( _ch == 1 && _bps ==  8 )
	{
		_smp_head = head_size  / 1;
		_smp_body = body_size  / 1;
		_smp_tail = tail_size  / 1;
		sample_num      = work_size  / 1;
		work_size       = sample_num * 1;
		p1byte_data     = (u8 *)_p_smp;
		if( !_malloc_zero( (void **)&p1byte_work, work_size ) ) goto End;
		for( a = 0; a < sample_num; a++ )
		{
			b = (s32)( (f64)a * (f64)(_sps) / (f64)(new_sps) );
			p1byte_work[a] = p1byte_data[b];
		}

	}
	else
	// mono 16bit / stereo 8bit ========
	{

		_smp_head = head_size  / 2;
		_smp_body = body_size  / 2;
		_smp_tail = tail_size  / 2;
		sample_num      = work_size  / 2;
		work_size       = sample_num * 2;
		p2byte_data     = (u16 *)_p_smp;
		if( !_malloc_zero( (void **)&p2byte_work, work_size ) ) goto End;
		for( a = 0; a < sample_num; a++ )
		{
			b = (s32)( (f64)a * (f64)(_sps) / (f64)new_sps );
			p2byte_work[a] = p2byte_data[b];
		}

	}

	// release once.
	_free_null( (void **)&_p_smp );
	if( !_malloc_zero( (void **)&_p_smp, work_size ) ) goto End;

	if(      p4byte_work ) memcpy( _p_smp, p4byte_work, work_size );
	else if( p2byte_work ) memcpy( _p_smp, p2byte_work, work_size );
	else if( p1byte_work ) memcpy( _p_smp, p1byte_work, work_size );
	else goto End;

	// update.
	_sps = new_sps;

	b_ret = true;
End:

	if( !b_ret )
	{
		_free_null( (void **)&_p_smp );
		_smp_head = 0;
		_smp_body = 0;
		_smp_tail = 0;
	}

	_free_null( (void **)&p2byte_work  );
	_free_null( (void **)&p1byte_work  );
	_free_null( (void **)&p4byte_work  );

	return b_ret;
}

// convert..
bool pxPulse_PCM::Convert( s32 new_ch, s32 new_sps, s32 new_bps )
{

	if( !_Convert_ChannelNum     ( new_ch  ) ) return false;
	if( !_Convert_BitPerSample   ( new_bps ) ) return false;
	if( !_Convert_SamplePerSecond( new_sps ) ) return false;

	return true;
}

bool pxPulse_PCM::Copy( pxPulse_PCM *p_dst ) const
{
	if( !_p_smp ){ p_dst->Release(); return true; }

	if( !p_dst->Make( _ch, _sps, _bps, _smp_body ) ) return false;
	memcpy( p_dst->_p_smp, _p_smp, ( _smp_head + _smp_body + _smp_tail ) * _ch * _bps / 8 );
	return true;
}

bool pxPulse_PCM::Copy_( pxPulse_PCM *p_dst, s32 start, s32 end ) const
{
	s32 size, offset;

	if( _smp_head || _smp_tail ) return false;
	if( !_p_smp ){ p_dst->Release(); return true; }

//	*p_dst = *p_src;

	size   = ( end - start ) * _ch * _bps / 8;
	offset =         start   * _ch * _bps / 8;

	if( !p_dst->Make( _ch, _sps, _bps, end - start ) ) return false;

//	if( !_malloc_zero( (void**)&p_dst->p_smp, size ) ) return false;

	memcpy( p_dst->_p_smp, &_p_smp[ offset ], size );

//	p_dst->_smp_body = end - start;

	return true;
}


s32  pxPulse_PCM::get_ch      () const{ return _ch      ; };
s32  pxPulse_PCM::get_bps     () const{ return _bps     ; };
s32  pxPulse_PCM::get_sps     () const{ return _sps     ; };
s32  pxPulse_PCM::get_smp_body() const{ return _smp_body; };
s32  pxPulse_PCM::get_smp_head() const{ return _smp_head; };
s32  pxPulse_PCM::get_smp_tail() const{ return _smp_tail; };

const void *pxPulse_PCM::get_p_buf         () const{ return _p_smp; }
void       *pxPulse_PCM::get_p_buf_variable() const{ return _p_smp; }

f32 pxPulse_PCM::get_sec   () const
{
	return (f32)(_smp_body+_smp_head+_smp_tail) / (f32)_sps;
}

s32 pxPulse_PCM::get_buf_size() const
{
	return ( _smp_head + _smp_body + _smp_tail ) * _ch * _bps / 8;
}


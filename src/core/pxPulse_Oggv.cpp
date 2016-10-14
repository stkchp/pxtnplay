#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vorbis/vorbisfile.h>

#include "./pxPulse_Oggv.h"


typedef struct
{
    s8* p_buf;// ogg vorbis-data on memory.s
    s32 size ;//
    s32 pos  ;// reading position.
}
OVMEM;


// 4 callbacks below:

static size_t _mread( void *p, size_t size, size_t nmemb, void* p_void )
{
	OVMEM *pom = (OVMEM*)p_void;

	if( !pom                  ) return -1;
	if( pom->pos >= pom->size ) return  0;
	if( pom->pos == -1        ) return  0;

	s32 left = pom->size - pom->pos;

	if( (s32)(size * nmemb) >= left )
	{
		memcpy( p, &pom->p_buf[ pom->pos ], pom->size - pom->pos );
		pom->pos = pom->size;
		return left / size;
	}

	memcpy( p, &pom->p_buf[ pom->pos ], nmemb * size );
	pom->pos += ( nmemb * size );

	return nmemb;
}

static s32 _mseek( void* p_void, ogg_int64_t offset, s32 mode )
{
	s32   newpos;
	OVMEM *pom = (OVMEM*)p_void;

	if( !pom || pom->pos < 0 ) return -1;

	if( offset < 0 )
	{
		pom->pos = -1;
		return -1;
	}

	switch( mode )
	{
	case SEEK_SET: newpos =             (s32)offset; break;
	case SEEK_CUR: newpos = pom->pos  + (s32)offset; break;
	case SEEK_END: newpos = pom->size + (s32)offset; break;
	default: return -1;
	}
	if( newpos < 0 ) return -1;

	pom->pos = newpos;

	return 0;
}

static long int _mtell( void* p_void )
{
	OVMEM* pom = (OVMEM*)p_void;
	if( !pom ) return -1;
	return pom->pos;
}

static s32 _mclose_dummy( void* p_void )
{
	OVMEM* pom = (OVMEM*)p_void;
	if( !pom ) return -1;
	return 0;
}


bool pxPulse_Oggv::_SetInformation()
{
	bool b_ret = false;

	OVMEM ovmem;
	ovmem.p_buf = _p_data;
	ovmem.pos   =       0;
	ovmem.size  = _size  ;

	// set callback func.
	ov_callbacks   oc; 
	oc.read_func  = _mread       ;
	oc.seek_func  = _mseek       ;
	oc.close_func = _mclose_dummy;
	oc.tell_func  = _mtell       ;
  
	OggVorbis_File vf;
	
	vorbis_info*  vi;

	switch( ov_open_callbacks( &ovmem, &vf, NULL, 0, oc ) )
	{
	case OV_EREAD     : goto End; //{printf("A read from media returned an error.\n");exit(1);} 
	case OV_ENOTVORBIS: goto End; //{printf("Bitstream is not Vorbis data. \n");exit(1);}
	case OV_EVERSION  : goto End; //{printf("Vorbis version mismatch. \n");exit(1);}
	case OV_EBADHEADER: goto End; //{printf("Invalid Vorbis bitstream header. \n");exit(1);}
	case OV_EFAULT    : goto End; //{printf("Internal logic fault; indicates a bug or heap/stack corruption. \n");exit(1);}
	default:
		break;
	}

	vi = ov_info( &vf,-1 );

	_ch      = vi->channels;
	_sps2    = vi->rate    ;
	_smp_num = (s32)ov_pcm_total( &vf, -1 );
    
	// end.
	ov_clear( &vf );

	b_ret = true;

End:
    return b_ret;

}


/////////////////
// global
/////////////////


pxPulse_Oggv::pxPulse_Oggv()
{
	_p_data  = NULL;
	_ch      = 0;
	_sps2    = 0;
	_smp_num = 0;
	_size    = 0;
}

pxPulse_Oggv::~pxPulse_Oggv()
{
	Release();
}

void pxPulse_Oggv::Release()
{
	if( _p_data ) free( _p_data ); _p_data = NULL;
	_ch      = 0;
	_sps2    = 0;
	_smp_num = 0;
	_size    = 0;
}

bool pxPulse_Oggv::Load( const char* path )
{
	bool    b_ret  = false;
	pxwrDoc doc;

	if( !doc.Open_path( path, "rb" )        ) goto End;
	if( !( _size = doc.FileSize() )         ) goto End;
	if( !( _p_data = (s8*)malloc( _size ) ) ) goto End;
	if( !doc.r( _p_data, 1, _size )         ) goto End;
	if( !_SetInformation()                  ) goto End;

	b_ret = true;
End:

	if( !b_ret )
	{
		if( _p_data ) free( _p_data ); _p_data = NULL; _size = 0;
	}
	return b_ret;
}

bool pxPulse_Oggv::Save( const char* path ) const
{
	bool    b_ret  = false;
	pxwrDoc doc;

	if( !doc.Open_path( path, "wb" ) ) goto End;
	if( !doc.w( _p_data, 1,_size )   ) goto End;

	b_ret = true;
End:
	return b_ret;
}

bool pxPulse_Oggv::Decode( pxPulse_PCM * p_pcm ) const
{
	bool           b_ret  = false;

	OggVorbis_File vf;
	vorbis_info*   vi;
	ov_callbacks   oc; 

	OVMEM ovmem;

	ovmem.p_buf = _p_data;
	ovmem.pos   =       0;
	ovmem.size  = _size  ;

	// set callback func.
	oc.read_func  = _mread       ;
	oc.seek_func  = _mseek       ;
	oc.close_func = _mclose_dummy;
	oc.tell_func  = _mtell       ;
  
	switch( ov_open_callbacks( &ovmem, &vf, NULL, 0, oc ) )
	{
	case OV_EREAD     : goto End; //{printf("A read from media returned an error.\n");exit(1);} 
	case OV_ENOTVORBIS: goto End; //{printf("Bitstream is not Vorbis data. \n");exit(1);}
	case OV_EVERSION  : goto End; //{printf("Vorbis version mismatch. \n");exit(1);}
	case OV_EBADHEADER: goto End; //{printf("Invalid Vorbis bitstream header. \n");exit(1);}
	case OV_EFAULT    : goto End; //{printf("Internal logic fault; indicates a bug or heap/stack corruption. \n");exit(1);}
	default: break;
	}

	vi    = ov_info( &vf,-1 );

	
	s32 current_section;
	char pcmout[ 4096 ]; //take 4k out of the data segment, not the stack
	{
		s32 smp_num = (s32)ov_pcm_total( &vf, -1 );
		u32 bytes;

		bytes = vi->channels * 2 * smp_num;

		if( !p_pcm->Make( vi->channels, vi->rate, 16, smp_num ) ) goto End;
	}
	// decode..
	{
		s32 ret = 0;
		u8* p   = (u8*)p_pcm->get_p_buf_variable();
		do
		{
			ret = ov_read( &vf, pcmout, 4096, 0, 2, 1, &current_section );
			if( ret > 0 ) memcpy( p, pcmout, ret ); //fwrite( pcmout, 1, ret, of );
			p += ret;
		}
		while( ret );
	}
    
	// end.
	ov_clear( &vf );

	b_ret = true;

End:
	return b_ret;
}

bool pxPulse_Oggv::GetInfo( s32* p_ch, s32* p_sps, s32* p_smp_num )
{
//	const pxOGGVORBISSTRUCT* p_oggv = (const pxOGGVORBISSTRUCT*)h_oggv;
	if( !_p_data ) return false;

//	if( !p_oggv ) return false;

	if( p_ch      ) *p_ch      = _ch     ;
	if( p_sps     ) *p_sps     = _sps2   ;
	if( p_smp_num ) *p_smp_num = _smp_num;

	return true;
}

s32  pxPulse_Oggv::GetSize() const
{
	if( !_p_data ) return 0;
//	if( !h_oggv ) return 0;
//	const pxOGGVORBISSTRUCT* p_oggv = (const pxOGGVORBISSTRUCT*)h_oggv;
//	return sizeof(pxOGGVORBISSTRUCT) - sizeof(s8*) + p_oggv->size;
	return sizeof(s32)*4 + _size;
}

bool pxPulse_Oggv::Write( pxwrDoc *p_doc ) const
{
	if( !_p_data ) return false;
//	if( !h_oggv ) return false;
//	const pxOGGVORBISSTRUCT* p_oggv = (const pxOGGVORBISSTRUCT*)h_oggv;

	if( !p_doc->w( &_ch     , sizeof(s32),    1 ) ) return false;
	if( !p_doc->w( &_sps2   , sizeof(s32),    1 ) ) return false;
	if( !p_doc->w( &_smp_num, sizeof(s32),    1 ) ) return false;
	if( !p_doc->w( &_size   , sizeof(s32),    1 ) ) return false;
	if( !p_doc->w(  _p_data , sizeof(s8), _size ) ) return false;

	return true;
}

bool pxPulse_Oggv::Read( pxwrDoc *p_doc )
{
	bool  b_ret  = false;

//	pxOGGVORBISSTRUCT* p_oggv = NULL;

//	if( !(  p_oggv = (pxOGGVORBISSTRUCT*)malloc( sizeof(pxOGGVORBISSTRUCT) ) ) ) goto End;
//	memset( p_oggv, 0,                           sizeof(pxOGGVORBISSTRUCT) ); 

	if( !p_doc->r( &_ch     , sizeof(s32), 1 ) ) goto End;
	if( !p_doc->r( &_sps2   , sizeof(s32), 1 ) ) goto End;
	if( !p_doc->r( &_smp_num, sizeof(s32), 1 ) ) goto End;
	if( !p_doc->r( &_size   , sizeof(s32), 1 ) ) goto End;

	if( !_size ) goto End;

	if( !( _p_data = (s8*)malloc( _size ) ) ) goto End;
	if( !p_doc->r( _p_data, 1,    _size )   ) goto End;

	b_ret = true;
End:

	if( !b_ret )
	{
		if( _p_data ) free( _p_data ); _p_data = NULL; _size = 0;
	}

	return b_ret;
}

bool pxPulse_Oggv::Copy( pxPulse_Oggv *p_dst ) const
{
	p_dst->Release();
	if( !_p_data ) return true;

	if( !( p_dst->_p_data = (s8*)malloc( _size ) ) ) return false;
	memcpy( p_dst->_p_data, _p_data, _size );

	p_dst->_ch      = _ch     ;
	p_dst->_sps2    = _sps2   ;
	p_dst->_size    = _size   ;
	p_dst->_smp_num = _smp_num;

	return true;
}

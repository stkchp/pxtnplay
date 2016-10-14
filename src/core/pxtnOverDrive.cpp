// '12/03/03

#include <stdlib.h>
#include <string.h>

#include "./pxtnOverDrive.h"

pxtnOverDrive::pxtnOverDrive()
{
	_b_played = true;
}

pxtnOverDrive::~pxtnOverDrive()
{
}

f32 pxtnOverDrive::get_cut  ()const{ return _cut_f; }
f32 pxtnOverDrive::get_amp  ()const{ return _amp_f; }
s32 pxtnOverDrive::get_group()const{ return _group; }

void  pxtnOverDrive::Set( f32 cut, f32 amp, s32 group )
{
	_cut_f = cut  ;
	_amp_f = amp  ;
	_group = group;
}

bool pxtnOverDrive::get_played()const{ return _b_played; }
void pxtnOverDrive::set_played( bool b ){ _b_played = b; }
bool pxtnOverDrive::switch_played(){ _b_played = _b_played ? false : true; return _b_played; }

void pxtnOverDrive::Tone_Ready()
{
	_cut_16bit_top = (s32)( 32767 * ( 100 - _cut_f ) / 100 );
}

void pxtnOverDrive::Tone_Supple( s32 *group_smps ) const
{
	if( !_b_played ) return;
	s32 work = group_smps[ _group ];
	if(      work >  _cut_16bit_top ) work = (s32)(  _cut_16bit_top );
	else if( work < -_cut_16bit_top ) work = (s32)( -_cut_16bit_top );
	group_smps[ _group ] = (s32)( (f32)work * _amp_f );
}


// (8byte) =================
typedef struct
{
	u16 xxx  ;
	u16 group;
	f32 cut  ;
	f32 amp  ;
	f32 yyy  ;
}
_OVERDRIVESTRUCT;

bool pxtnOverDrive::Write( pxwrDoc *p_doc ) const
{
	_OVERDRIVESTRUCT over;
	s32              size;

	memset( &over, 0, sizeof( _OVERDRIVESTRUCT ) );
	over.cut   = _cut_f;
	over.amp   = _amp_f;
	over.group = (u16)_group;

	// dela ----------
	size = sizeof( _OVERDRIVESTRUCT );
	if( !p_doc->w( &size, sizeof(u32), 1 ) ) return false;
	if( !p_doc->w( &over, size,        1 ) ) return false;

	return true;
}

// 読み込み
bool pxtnOverDrive::Read( pxwrDoc *p_doc, bool *pb_new_fmt )
{
	_OVERDRIVESTRUCT over;
	s32              size;

	memset( &over, 0, sizeof(_OVERDRIVESTRUCT) );
	if( !p_doc->r( &size, 4,                        1 ) ) return false;
	if( !p_doc->r( &over, sizeof(_OVERDRIVESTRUCT), 1 ) ) return false;

	if( over.xxx                         ) return false;
	if( over.yyy                         ) return false;
	if( over.cut > TUNEOVERDRIVE_CUT_MAX || over.cut < TUNEOVERDRIVE_CUT_MIN ) return false;
	if( over.amp > TUNEOVERDRIVE_AMP_MAX || over.amp < TUNEOVERDRIVE_AMP_MIN ) return false;

	_cut_f = over.cut  ;
	_amp_f = over.amp  ;
	_group = over.group;

	return true;
}

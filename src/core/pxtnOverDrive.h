// '12/03/03

#ifndef pxtnOverDrive_H
#define pxtnOverDrive_H

#include "./pxTypedef.h"

#include <pxwrDoc.h>

#define TUNEOVERDRIVE_CUT_MAX       99.9f
#define TUNEOVERDRIVE_CUT_MIN       50.0f
#define TUNEOVERDRIVE_AMP_MAX        8.0f
#define TUNEOVERDRIVE_AMP_MIN        0.1f

#define TUNEOVERDRIVE_DEFAULT_CUT   90.0f
#define TUNEOVERDRIVE_DEFAULT_AMP    2.0f

class pxtnOverDrive
{
private:

	bool _b_played;

	s32  _group   ;
	f32  _cut_f   ;
	f32  _amp_f   ;

	s32  _cut_16bit_top;
public :


	 pxtnOverDrive();
	~pxtnOverDrive();

	void Tone_Ready();
	void Tone_Supple( s32 *group_smps ) const;

	bool Write( pxwrDoc *p_doc ) const;
	bool Read ( pxwrDoc *p_doc, bool *pb_new_fmt );


	f32  get_cut  ()const;
	f32  get_amp  ()const;
	s32  get_group()const;

	void Set( f32 cut, f32 amp, s32 group );

	bool get_played()const;
	void set_played( bool b );
	bool switch_played();

};

#endif

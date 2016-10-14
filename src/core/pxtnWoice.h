// '12/03/03

#ifndef pxtnWoice_H
#define pxtnWoice_H

#include <pxwrDoc.h>

#include "./pxPulse_Noise.h"
#include "./pxPulse_NoiseBuilder.h"
#include "./pxPulse_PCM.h"
#include "./pxPulse_Oggv.h"

#define MAX_TUNEWOICENAME   16 // fixture.

#define MAX_UNITCONTROLVOICE 2 // max-woice per unit

#define pxtnBUFSIZE_TIMEPAN  0x40

#define PTV_VOICEFLAG_WAVELOOP   0x00000001
#define PTV_VOICEFLAG_SMOOTH     0x00000002
#define PTV_VOICEFLAG_BEATFIT    0x00000004
#define PTV_VOICEFLAG_UNCOVERED  0xfffffff8

#define PTV_DATAFLAG_WAVE        0x00000001
#define PTV_DATAFLAG_ENVELOPE    0x00000002
#define PTV_DATAFLAG_UNCOVERED   0xfffffffc

enum pxtnWOICETYPE
{
	pxtnWOICE_None = 0,
	pxtnWOICE_PCM ,
	pxtnWOICE_PTV ,
	pxtnWOICE_PTN ,
	pxtnWOICE_OGGV,
};

enum pxtnVOICETYPE
{
	pxtnVOICE_Coodinate = 0,
	pxtnVOICE_Overtone ,
	pxtnVOICE_Noise    ,
	pxtnVOICE_Sampling ,
	pxtnVOICE_OggVorbis,
};

typedef struct
{
	s32 smp_head_w ;
	s32 smp_body_w ;
	s32 smp_tail_w ;
	u8* p_smp_w    ;

	u8* p_env      ;
	s32 env_size   ;
	s32 env_release;
}
pxtnVOICEWORK;

typedef struct
{
	s32     fps     ;
	s32     head_num;
	s32     body_num;
	s32     tail_num;
	sPOINT* points;
}
pxtnVOICEENVELOPE;

typedef struct
{
	s32     num   ;
	s32     reso  ; // COODINATERESOLUTION
	sPOINT* points;
}
pxtnVOICEWAVE;

typedef struct
{
	s32 basic_key  ;
	s32 volume     ;
	s32 pan        ;
	f32 correct    ;
	u32 voice_flags;
	u32 data_flags ;
					  
	pxtnVOICETYPE     type       ;
	pxPulse_PCM       *p_pcm     ;
	pxPulse_Oggv      *p_oggv    ;
	pxPulse_Noise     *p_ptn     ;

	pxtnVOICEWAVE     wave       ;
	pxtnVOICEENVELOPE envelope   ;
}
pxtnVOICEUNIT;

typedef struct
{
	f64 smp_pos    ;       
	f32 offset_freq;
	s32 env_volume ;
	s32 life_count ;
	s32 on_count   ;

	s32 smp_count  ;
	s32 env_start  ;
	s32 env_pos    ;
	s32 env_release_clock;

	s32 smooth_volume;
}
pxtnVOICETONE;


class pxtnWoice
{
private:

	s32           _voice_num;
	char          _name[ MAX_TUNEWOICENAME + 1 ];
	pxtnWOICETYPE _type;
	pxtnVOICEUNIT *_vcs ;
	pxtnVOICEWORK *_vws ;

	f32           _x3x_correct;
	s32           _x3x_basic_key; // correct old-fmt when key-event

public :
	 pxtnWoice();
	~pxtnWoice();

	s32           get_voice_num    () const;
	f32           get_x3x_correct  () const;
	s32           get_x3x_basic_key() const;
	pxtnWOICETYPE get_type         () const;
	const char    *get_name        () const;
	const pxtnVOICEUNIT *get_voice         ( s32 idx ) const;
	pxtnVOICEUNIT       *get_voice_variable( s32 idx );

	const pxtnVOICEWORK *get_work ( s32 idx ) const;

	void set_name( const char *name );

	bool Voice_Allocate( s32 voice_num );
	void Voice_Release ();
	bool Copy( pxtnWoice *p_dst ) const;
	void Slim();

	bool Load( const char *path, pxtnWOICETYPE type, bool *pb_new_fmt );

	
	bool PTV_Write   ( pxwrDoc *p_doc, s32 *p_total     ) const;
	bool PTV_Read    ( pxwrDoc *p_doc, bool *pb_new_fmt );
	bool PTV_Save    ( const char* path                 ) const;

	bool io_matePCM_w(  pxwrDoc *p_doc ) const;
	bool io_matePCM_r(  pxwrDoc *p_doc, bool *pb_new_fmt );

	bool io_matePTN_w(  pxwrDoc *p_doc ) const;
	bool io_matePTN_r(  pxwrDoc *p_doc, bool *pb_new_fmt );

	bool io_matePTV_w(  pxwrDoc *p_doc ) const;
	bool io_matePTV_r(  pxwrDoc *p_doc, bool *pb_new_fmt );

	bool io_mateOGGV_w( pxwrDoc *p_doc ) const;
	bool io_mateOGGV_r( pxwrDoc *p_doc, bool *pb_new_fmt );

	bool Tone_Ready_sample  ( const pxPulse_NoiseBuilder *ptn_bldr  );
	bool Tone_Ready_envelope( s32 sps );
	bool Tone_Ready         ( const pxPulse_NoiseBuilder *ptn_bldr, s32 sps );
};

#endif

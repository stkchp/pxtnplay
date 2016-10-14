#ifndef pxPulse_PCM_H
#define pxPulse_PCM_H

#include "./pxTypedef.h"

class pxPulse_PCM
{
private:
	s32 _ch      ;
	s32 _sps     ;
	s32 _bps     ;
	s32 _smp_head; // no use. 0
	s32 _smp_body;
	s32 _smp_tail; // no use. 0
	u8* _p_smp   ;

	bool _Convert_ChannelNum     ( s32 new_ch  );
	bool _Convert_BitPerSample   ( s32 new_bps );
	bool _Convert_SamplePerSecond( s32 new_sps );

public:

	 pxPulse_PCM();
	~pxPulse_PCM();

	bool Make   ( s32 ch, s32 sps, s32 bps, s32 sample_num );
	void Release();
	bool Load   ( const char* path );
	bool Save   ( const char* path, const char* pstrLIST ) const;
	bool Convert( s32  new_ch, s32 new_sps, s32 new_bps );
	bool Copy   ( pxPulse_PCM *p_dst ) const;
	bool Copy_  ( pxPulse_PCM *p_dst, s32 start, s32 end ) const;

	void *Devolve_SamplingBuffer();

	f32 get_sec   () const;

	s32 get_ch      () const;
	s32 get_bps     () const;
	s32 get_sps     () const;
	s32 get_smp_body() const;
	s32 get_smp_head() const;
	s32 get_smp_tail() const;

	s32 get_buf_size() const;

	const void *get_p_buf         () const;
	void       *get_p_buf_variable() const;

};

#endif

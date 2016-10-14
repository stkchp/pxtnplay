#include <stdio.h>
#include <math.h>

#include "./pxPulse_Oscillator.h"

pxPulse_Oscillator::pxPulse_Oscillator()
{
	_volume      =    0;
	_p_point     = NULL;
	_sample_num  =    0;
	_point_num   =    0;
	_point_reso  =    0;
}

void pxPulse_Oscillator::ReadyGetSample( sPOINT *p_point, s32 point_num, s32 volume, s32 sample_num, s32 point_reso )
{
	_volume      = volume;
	_p_point     = p_point;
	_sample_num  = sample_num;
	_point_num   = point_num;
	_point_reso  = point_reso;
}

f64 pxPulse_Oscillator::GetOneSample_Overtone( s32 index )
{
	s32   o;
	f64 work_double;
	f64 pi = 3.1415926535897932;
	f64 sss;

	work_double = 0;
	for( o = 0; o < _point_num; o++ )
	{
		sss          = 2 * pi * ( _p_point[ o ].x ) * index / _sample_num;
		work_double += ( sin( sss ) * (f64)_p_point[ o ].y / ( _p_point[ o ].x ) / 128 );
	}
	work_double = work_double * _volume / 128;

	return work_double;
}

f64 pxPulse_Oscillator::GetOneSample_Coodinate( s32 index )
{
	s32 i;
	s32 c;
	s32 x1, y1, x2, y2;
	s32 w, h;
	f64 work;
	
	i = _point_reso * index / _sample_num;

	// 対象の２ポイントを探す
	c = 0;
	while( c < _point_num )
	{
		if( _p_point[ c ].x > i ) break;
		c++;
	}

	//末端
	if( c == _point_num )
	{
		x1 = _p_point[ c - 1 ].x;
		y1 = _p_point[ c - 1 ].y;
		x2 = _point_reso;
		y2 = _p_point[   0   ].y;
	}
	else
	{
		if( c ){
			x1 = _p_point[ c - 1 ].x;
			y1 = _p_point[ c - 1 ].y;
			x2 = _p_point[   c   ].x;
			y2 = _p_point[   c   ].y;
		}else{
			x1 = _p_point[   0   ].x;
			y1 = _p_point[   0   ].y;
			x2 = _p_point[   0   ].x;
			y2 = _p_point[   0   ].y;
		}
	}

	w = x2 - x1;
	i =  i - x1;
	h = y2 - y1;

	if( i ) work = (f64)y1 + (f64)h * (f64)i / (f64)w;
	else    work = y1;

	return work * _volume / 128 /128 ;

}

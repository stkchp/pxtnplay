#ifndef pxtnEvelist_H
#define pxtnEvelist_H

#include <pxwrDoc.h>

enum
{
	EVENTKIND_NULL  = 0 ,//  0

	EVENTKIND_ON        ,//  1
	EVENTKIND_KEY       ,//  2
	EVENTKIND_PAN_VOLUME,//  3
	EVENTKIND_VELOCITY  ,//  4
	EVENTKIND_VOLUME    ,//  5
	EVENTKIND_PORTAMENT ,//  6
	EVENTKIND_BEATCLOCK ,//  7
	EVENTKIND_BEATTEMPO ,//  8
	EVENTKIND_BEATNUM   ,//  9
	EVENTKIND_REPEAT    ,// 10
	EVENTKIND_LAST      ,// 11
	EVENTKIND_VOICENO   ,// 12
	EVENTKIND_GROUPNO   ,// 13
	EVENTKIND_CORRECT   ,// 14
	EVENTKIND_PAN_TIME  ,// 15

	EVENTKIND_NUM       ,// 16
};

#define EVENTDEFAULT_VOLUME       104
#define EVENTDEFAULT_VELOCITY     104
#define EVENTDEFAULT_PAN_VOLUME    64
#define EVENTDEFAULT_PAN_TIME      64
#define EVENTDEFAULT_PORTAMENT      0
#define EVENTDEFAULT_VOICENO        0
#define EVENTDEFAULT_GROUPNO        0
#define EVENTDEFAULT_KEY       0x6000
#define EVENTDEFAULT_BASICKEY  0x4500 // 4A(440Hz?)
#define EVENTDEFAULT_CORRECT     1.0f

#define EVENTDEFAULT_BEATNUM        4
#define EVENTDEFAULT_BEATTEMPO    120
#define EVENTDEFAULT_BEATCLOCK    480

typedef struct EVERECORD
{
	u8         kind    ;
	u8         unit_no ;
	u8         reserve1;
	u8         reserve2;
	s32        value   ;
	s32        clock   ;
	EVERECORD* prev    ;
	EVERECORD* next    ;
}
EVERECORD;

//--------------------------------

class pxtnEvelist
{

private:

	s32        _max_num;
	EVERECORD* _recs   ;
	EVERECORD* _start  ;
	s32        _linear ;

	EVERECORD* _p_x4x_rec;

	pxtnEvelist(              const pxtnEvelist &src  ){               } // copy
	pxtnEvelist & operator = (const pxtnEvelist &right){ return *this; } // substitution

	void _SetRecord( EVERECORD* p_rec, EVERECORD* prev, EVERECORD* next, s32 clock, u8 unit_no, u8 kind, s32 value );
	void _CutRecord( EVERECORD* p_rec );

public:

	void Release();
	void Clear  ();

	 pxtnEvelist();
	~pxtnEvelist();

	bool Allocate( s32 max_num );

	s32 get_Num_Max  () const;
	s32 get_Max_Clock() const;
	s32 get_Count(    ) const;
	s32 get_Count(                                     u8 kind, s32 value ) const;
	s32 get_Count(                         u8 unit_no                     ) const;
	s32 get_Count(                         u8 unit_no, u8 kind            ) const;
	s32 get_Count( s32 clock1, s32 clock2, u8 unit_no                     ) const;
	s32 get_Value( s32 clock ,             u8 unit_no, u8 kind            ) const;

	const EVERECORD* get_Records( void ) const;

	bool Record_Add_i( s32 clock,          u8 unit_no, u8 kind, s32 value   );
	bool Record_Add_f( s32 clock,          u8 unit_no, u8 kind, f32 value_f );

	bool Linear_Start( void );
	void Linear_Add_i( s32 clock,          u8 unit_no, u8 kind, s32 value   );
	void Linear_Add_f( s32 clock,          u8 unit_no, u8 kind, f32 value_f );
	void Linear_End(   bool b_connect );

	s32 Record_Clock_Shift(   s32 clock,  s32 shift , u8 unit_no  ); // can't be under 0.
	s32 Record_Value_Set(     s32 clock1, s32 clock2, u8 unit_no, u8 kind, s32 value );
	s32 Record_Value_Change(  s32 clock1, s32 clock2, u8 unit_no, u8 kind, s32 value );
	s32 Record_Value_Omit(                                        u8 kind, s32 value );
	s32 Record_Value_Replace(                                     u8 kind, s32 old_value, s32 new_value ); 
	s32 Record_Delete(        s32 clock1, s32 clock2, u8 unit_no, u8 kind            );
	s32 Record_Delete(        s32 clock1, s32 clock2, u8 unit_no                     );

	s32 Record_UnitNo_Miss(                           u8 unit_no                     ); // delete event has the unit-no
	s32 Record_UnitNo_Set(                            u8 unit_no                     ); // set the unit-no
	s32 Record_UnitNo_Replace(                        u8 old_u, u8 new_u             ); // exchange unit


	bool io_Write( pxwrDoc *p_doc, s32 rough        ) const;
	bool io_Read ( pxwrDoc *p_doc, bool *pb_new_fmt ); 
	s32  io_Read_EventNum( pxwrDoc *p_doc ) const;


	bool x4x_Read_Start(   void );
	void x4x_Read_NewKind( void );
	void x4x_Read_Add(     s32 clock, u8 unit_no, u8 kind, s32 value );

	bool io_Unit_Read_x4x_EVENT( pxwrDoc *p_doc, bool bTailAbsolute, bool bCheckRRR, bool *pb_new_fmt );
	s32 io_Read_x4x_EventNum( pxwrDoc *p_doc ) const;

};

bool Evelist_Kind_IsTail( s32 kind );

#endif

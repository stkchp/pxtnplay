#ifndef pxTypedef_H
#define pxTypedef_H

#include <cstdint>

#ifndef SAFE_DELETE
#define SAFE_DELETE(p){ if (p) { delete (p); (p)=NULL; } }
#endif

using u8  = std::uint8_t ;
using s8  = std::int8_t  ;
using u16 = std::uint16_t;
using s16 = std::int16_t ;
using u32 = std::uint32_t;
using s32 = std::int32_t ;

using f32 = float ;
using f64 = double;

typedef struct
{
	f32 l;
	f32 t;
	f32 r;
	f32 b;
}
fRECT;

typedef struct
{
	s32 l;
	s32 t;
	s32 r;
	s32 b;
}
sRECT;

typedef struct
{
	f32 x;
	f32 y;
}
fPOINT;

typedef struct 
{
	s32 x;
	s32 y;
}
sPOINT;

#endif

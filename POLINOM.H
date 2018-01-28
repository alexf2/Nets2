#if !defined(_POLY_)
#define _POLY_

/*------------------------------------------------------------*/
WORD CodeByteToWord( BYTE data );
BYTE CodeWordToByte( WORD data, BYTE& flag );

BYTE CodeFourBits( BYTE data );
BYTE DeCodeFourBits( BYTE data );
/*------------------------------------------------------------*/

struct TPacket;

void CodePacket( TPacket* pPck );
bool DecodePacket( TPacket* pPck );

#endif
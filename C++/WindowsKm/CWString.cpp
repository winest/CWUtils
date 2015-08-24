#include "MyString.h"

namespace KmUtils
{

#ifdef __cplusplus
extern "C" {
#endif

LONG UStringCompareUString( CONST PUNICODE_STRING aSrc , CONST PUNICODE_STRING aDst , ULONG aCompareLen , BOOLEAN aCaseSensitive )
{
    WCHAR wSrc , wDst;
    for ( ULONG i = 0 ; i < aCompareLen ; i++ )
    {
        if ( aCaseSensitive )
        {
            wSrc = aSrc->Buffer[i];
            wDst = aDst->Buffer[i];
        }
        else
        {
            wSrc = TO_LOWER( aSrc->Buffer[i] );
            wDst = TO_LOWER( aDst->Buffer[i] );
        }
            
        if ( wSrc != wDst )
        {
            return ( aSrc->Buffer[i] < aDst->Buffer[i] ) ? -1 : 1;
        }
    }
    return 0;
}

#ifdef __cplusplus
}
#endif

}   //End of namespace KmUtils

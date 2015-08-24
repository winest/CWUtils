#include <Windows.h>
#include <string>
#include <list>
#include <map>

namespace CWUtils
{

#ifdef __cplusplus
extern "C" {
#endif

BOOL GetIniSectionNames( CONST WCHAR * aIniPath , std::list<std::wstring> & aSectionNames );
BOOL GetIniSectionValues( const WCHAR * aIniPath , const WCHAR * aSectionName , std::map<std::wstring,std::wstring> & aKeyVal );

#ifdef __cplusplus
}
#endif

}   //End of namespace CWUtils
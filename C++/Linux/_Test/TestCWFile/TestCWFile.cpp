#include "stdafx.h"
#include "CWFile.h"

using namespace std;

int main()
{
	BOOL bRet = CWUtils::IsFileExist( "123.txt" );
	printf( "bRet=%d\n" , bRet );


	CWUtils::CFile file;
	file.Open( "123.txt" , CWUtils::FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST , true , true , true );

	const char * pData = "Content for testing";
	file.Write( (const UCHAR *)pData , strlen(pData) );
	file.Close();
	return 0;
}

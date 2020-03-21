#include "stdafx.h"
#include <string>
#include <unistd.h>
#include "CWTime.h"

using namespace std;

int main()
{
    CWUtils::CStopWatch stopWatch;
    stopWatch.Start();
    printf( "Wait 3 seconds\n" );
    sleep( 3 );
    stopWatch.Stop();

    string strTime;
    CWUtils::FormatTime( stopWatch.GetIntervalInMilli(), strTime );

    printf( "%llu milli-seconds (%s)\n", stopWatch.GetIntervalInMilli(), strTime.c_str() );

    timespec timeInt;
    stopWatch.GetInterval( timeInt );
    printf( "%us %uns\n", timeInt.tv_sec, timeInt.tv_nsec );
    return 0;
}

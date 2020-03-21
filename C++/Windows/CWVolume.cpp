#include "stdafx.h"
#include "CWVolume.h"
#include "CWGeneralUtils.h"
using std::wstring;

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif


//Return the bitmap of logical drives, the least significant bit represents drive A
DWORD GetVolumes()
{
    return GetLogicalDrives();
}

//Get exist logical drive letters, aBuf will look like "C:\(NULL)D:\(NULL)E:\(NULL)..."
//Return how many logical drives are found
INT GetVolumesPath( std::wstring & aBuf )
{
    WCHAR wzBuf[( 4 * 26 ) + 1] = { 0 };
    DWORD written = GetLogicalDriveStringsW( _countof( wzBuf ) - 1, wzBuf );
    aBuf.assign( wzBuf, written );
    return written / 4;
}

//Retrieves the name of a volume on a computer(GUID path)(GUID stands for Global Unique Identifier)
//GUID path, for example, "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)"
//aBuf will be like "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)"
//aBuf should have enough space(volume counts * 50 WCHARs) to receive volume names
//Return how many volumes are found
INT GetVolumesGuidPath( std::wstring & aBuf )
{
    int count = 0;
    WCHAR name[50];
    aBuf.clear();

    HANDLE hFind = FindFirstVolumeW( name, 50 );
    if ( hFind == INVALID_HANDLE_VALUE )
    {
        ShowDebugMsg( L"FindFirstVolumeW() failed when CWVolume->GetVolumesGuidPath()" );
        return 0;
    }

    do
    {
        aBuf.append( name, 50 );
        count++;
    } while ( FindNextVolumeW( hFind, name, MAX_PATH ) );

    DWORD err = GetLastError();
    if ( err != ERROR_NO_MORE_FILES )
    {
        ShowDebugMsg( L"FindNextVolumeW() failed when CWVolume->GetVolumesGuidPath()" );
    }
    else
    {
    }

    FindVolumeClose( hFind );

    return count;
}






//Retrieves a list of drive paths for the specified volume
//aVolumePath will look like "C:\(NULL)D:\(NULL)"
//Return number of drive letters found if successful, 0 otherwise
INT GetVolumePathFromGuidPath( CONST WCHAR * aVolumeGuidPath, std::wstring & aVolumePath )
{
    DWORD writtenLen;
    WCHAR wzVolumePath[( 4 * 26 ) + 1] = { 0 };
    if ( GetVolumePathNamesForVolumeNameW( aVolumeGuidPath, wzVolumePath, _countof( wzVolumePath ), &writtenLen ) )
    {
        aVolumePath.assign( wzVolumePath, _countof( wzVolumePath ) );
        return writtenLen / 4;
    }
    else
    {
        ShowDebugMsg( L"GetVolumePathNamesForVolumeNameW() failed when CWVolume->GetVolumePathFromGuidPath()" );
        return 0;
    }
}

//aMountPath can be drive letter, GUID path, or mounted folder path, all of them must end with backslash
//aGuidPath will look like "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)"
//Return TRUE if successful, FALSE otherwise
BOOL GetGuidPathFromMountPath( CONST WCHAR * aMountPath, std::wstring & aGuidPath )
{
    WCHAR wzGuidPath[50] = { 0 };
    if ( GetVolumeNameForVolumeMountPointW( aMountPath, wzGuidPath, 50 ) )
    {
        aGuidPath = wzGuidPath;
        return TRUE;
    }
    else
    {
        ShowDebugMsg( L"GetVolumeNameForVolumeMountPointW() when CWVolume->GetGuidPathFromMountPath()" );
        return FALSE;
    }
}

//Retrieves the volume mount point where aFileOrDirPath is mounted
//aFileOrDirPath can be absolute or relative path
//aMountPath will be written as the path where aFileOrDirPath is mounted
//aMountPathLen is the length of aMountPath excluding terminating null
//Return TRUE if successful, FALSE otherwise
BOOL GetMountPathFromFileOrDirPath( CONST WCHAR * aFileOrDirPath, std::wstring & aMountPath )
{
    WCHAR wzMountPath[MAX_PATH] = { 0 };
    if ( GetVolumePathNameW( aFileOrDirPath, wzMountPath, _countof( wzMountPath ) ) )
    {
        aMountPath = wzMountPath;
        return TRUE;
    }
    else
    {
        ShowDebugMsg( L"GetVolumePathNameW() failed when CWVolume->GetMountPathFromFileOrDirPath()" );
        return FALSE;
    }
}

//Retrieves the name of a mounted folder on the specified volume
//aMountedFolders will be the name of mounted folders on the specified volume
//aMountedFolders should have enough space([row][column] = [count of folders][MAX_PATH+1] WCHARs)
//Return how many folders mounted on the specific volume if successful, 0 otherwise
INT GetMountedFoldersFromGuidPath( CONST WCHAR * aVolumeGuidPath, WCHAR ** aMountedFolders )
{
    int count = 0;
    DWORD err;

    HANDLE hFindMount = FindFirstVolumeMountPointW( aVolumeGuidPath, aMountedFolders[count], MAX_PATH );
    if ( hFindMount == INVALID_HANDLE_VALUE )
    {
        ShowDebugMsg( L"FindFirstVolumeMountPointW() failed when CWVolume->GetMountedFoldersFromGuidPath()" );
        return 0;
    }
    else
    {
        count++;

        while ( FindNextVolumeMountPointW( hFindMount, aMountedFolders[count], MAX_PATH ) )
        {
            count++;
        }

        err = GetLastError();
        if ( err != ERROR_NO_MORE_FILES )
        {
            ShowDebugMsg( L"FindNextVolumeMountPointW failed when CWVolume->GetMountedFoldersFromGuidPath()" );
        }
        else
        {
        }

        FindVolumeMountPointClose( hFindMount );
    }

    return count;
}






//aDrivePath should be like "C:\" or "\\?\C:\"
//The type will be written to aVolumeType is it's not NULL, therefore it show have enough space
//Return the type in UINT defined in WinBase.h
UINT GetVolumeType( CONST WCHAR * aVolumePath, std::wstring & aVolumeType )
{
    UINT type = GetDriveTypeW( aVolumePath );
    switch ( type )
    {
        case DRIVE_UNKNOWN:    //The drive type cannot be determined
        {
            aVolumeType = L"Unknown";
            break;
        }
        case DRIVE_NO_ROOT_DIR:    //The root path is invalid; for example, there is no volume mounted at the specified path
        {
            aVolumeType = L"Invalid path";
            break;
        }
        case DRIVE_REMOVABLE:    //The drive has removable media; for example, a floppy drive, thumb drive, or flash card reader
        {
            aVolumeType = L"Removable media";
            break;
        }
        case DRIVE_FIXED:    //The drive has fixed media; for example, a hard drive or flash drive
        {
            aVolumeType = L"Fixed media";
            break;
        }
        case DRIVE_REMOTE:    //The drive is a remote (network) drive
        {
            aVolumeType = L"Remote drive";
            break;
        }
        case DRIVE_CDROM:    //The drive is a CD-ROM drive
        {
            aVolumeType = L"CD-ROM";
            break;
        }
        case DRIVE_RAMDISK:    //The drive is a RAM disk
        {
            aVolumeType = L"RAM disk";
            break;
        }
        default:
        {
            aVolumeType = L"Error";
            break;
        }
    }

    return type;
}

//aVolumePath should be like "C:\" or "\\?\C:\"
//aVolumeName will be written by drive's name, aVolumeNameLen is the buffer length excluding '\0'
//aFs will be written by file system format, aFsLen is the buffer length excluding '\0', aFsFlag show the privilege
//aSerial show the serial of the drive OS assigned, not serial assigned by manufacture(use Win32_PhysicalMedia)
//aMaxFileLen is the max filename length this drive support, filename is the one between backslashes
//aFsFlag, aSerial, and aMaxFileLen are optional
//Return TRUE if successful, FALSE otherwise
BOOL GetVolumeInfo( CONST WCHAR * aVolumePath,
                    std::wstring & aVolumeName,
                    std::wstring & aFs,
                    DWORD * aFsFlag,
                    DWORD * aSerial,
                    DWORD * aMaxFileLen )
{
    WCHAR wzVolumeName[MAX_PATH] = { 0 };
    WCHAR wzFs[MAX_PATH] = { 0 };
    if ( GetVolumeInformationW( aVolumePath, wzVolumeName, _countof( wzVolumeName ), aSerial, aMaxFileLen, aFsFlag,
                                wzFs, _countof( wzFs ) ) )
    {
        aVolumeName = wzVolumeName;
        aFs = wzFs;
        return TRUE;
    }
    else
    {
        ShowDebugMsg( L"Error: GetVolumeInformationW()" );
        return FALSE;
    }
}

/*
//Support from Windows Vista
//aVolumeName will be written by drive's name, aVolumeNameLen is the buffer length excluding '\0'
//aFs will be written by file system format, aFsLen is the buffer length excluding '\0', aFsFlag show the privilege
//aSerial show the serial of the drive OS assigned, not serial assigned by manufacture(use Win32_PhysicalMedia)
//aMaxFileLen is the max filename length this drive support, filename is the one between backslashes
//aFsFlag, aSerial, and aMaxFileLen are optional
//Return TRUE if successful, FALSE otherwise
BOOL GetVolumeInfo( HANDLE hFile , WCHAR * aVolumeName , DWORD aVolumeNameLen ,
                               WCHAR * aFs , DWORD aFsLen , DWORD * aFsFlag ,
                               DWORD * aSerial , DWORD * aMaxFileLen )
{
    if ( GetVolumeInformationByHandleW( hFile , aVolumeName , aVolumeNameLen + 1 ,
                                        aSerial , aMaxFileLen , FsFlag , aFs , aFsLen + 1 ) )
    {
        return TRUE;
    }
    else
    {
        ShowDebugMsg( L"Error: GetVolumeInformationByHandleW()" );
        return FALSE;
    }
}
*/

//Get free space, total space for all users, or available free space for current user on the specific disk in bytes
//You can simply get one space information by setting others to NULL
//Return TRUE if successful, FALSE otherwise
BOOL GetVolumeSpace( CONST WCHAR * aVolumePath,
                     ULARGE_INTEGER * aFree,
                     ULARGE_INTEGER * aTotal,
                     ULARGE_INTEGER * aCurrAvailable )
{
    if ( GetDiskFreeSpaceExW( aVolumePath, aCurrAvailable, aTotal, aFree ) )
    {
        return TRUE;
    }
    else
    {
        ShowDebugMsg( L"Error: GetDiskFreeSpaceExW()" );
        return FALSE;
    }
}




//aVolumePath can be "C:\" or "C:\Windows\" that must end with backslash
//Return TRUE if successful, FALSE otherwise
BOOL SetVolumeName( WCHAR * aVolumePath, WCHAR * aNewName )
{
    if ( SetVolumeLabelW( aVolumePath, aNewName ) )
    {
        return TRUE;
    }
    else
    {
        ShowDebugMsg( L"Error: SetVolumeLabelW()" );
        return FALSE;
    }
}

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
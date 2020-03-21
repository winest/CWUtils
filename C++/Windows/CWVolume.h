#pragma once

/*
 * Copyright (c) 2009-2020, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their
 * programming. It should be very easy to port them to other projects or
 * learn how to implement things on different languages and platforms.
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

/*
 * Hard disk can be divided in to several partitions.
 *
 * Volume is a single accessible storage area with a single file system, typically(not necessarily) resident
 * on a single partition of a hard disk. OS can recognize a partition without recognizing any volume associated
 * with it, as when the OS cannot interpret the filesystem stored there. For example, a partition with ext3
 * filesystem on Windows.
 *
 * Drive is a general term, but logical drive usually synonymous with volume.
 *
 * I make some naming convention in CWVolume:
 *     VolumePath is regard as a drive letter end with backslash, for example, "C:\"
 *     VolumeName is regard as a logical drive's label, for example, "C:\" may have the label "SYSTEM"
 *     GUID stands for Global Unique Identifier, and GuidPath here looks like "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\"
 *     MountPath can be drive letter, GUID path, or mounted folder path, all of them must end with backslash.
 *
 * To determine whether it's a USB device, refer to SetupDiGetDeviceRegistryProperty().
 *
 * Some other functions not implemented are: SetVolumeMountPoint() and Win32_PhysicalMedia().
 */

#pragma warning( push, 0 )
#include <Windows.h>
#include <string>
#pragma warning( pop )

namespace CWUtils
{
#ifdef __cplusplus
extern "C" {
#endif

//Return the bitmap of logical drives, the least significant bit represents drive A
DWORD GetVolumes();
//Get exist logical drive letters, aBuf will look like "C:\(NULL)D:\(NULL)E:\(NULL)..."
//aBuf should have enough space(>=104 WCHARs) to receive these letters
//Return how many logical drives are found
INT GetVolumesPath( std::wstring & aBuf );
//Retrieves the name of a volume on a computer(GUID path)(GUID stands for Global Unique Identifier)
//GUID path, for example, "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)"
//aBuf will be like "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)"
//aBuf should have enough space(volume counts * 50 WCHARs) to receive volume names
//Return how many volumes are found
INT GetVolumesGuidPath( std::wstring & aBuf );




//Retrieves a list of drive paths for the specified volume
//aVolumePath will look like "C:\(NULL)D:\(NULL)"
//Return number of drive letters found if successful, 0 otherwise
INT GetVolumePathFromGuidPath( CONST WCHAR * aVolumeGuidPath, std::wstring & aVolumePath );
//aMountPath can be drive letter, GUID path, or mounted folder path, all of them must end with backslash
//aGuidPath will look like "\\?\Volume{183263e9-5394-11df-a364-806d6172696f}\(NULL)"
//aGuidPath should have enough space(>=50 WCHARs)
//Return TRUE if successful, FALSE otherwise
BOOL GetGuidPathFromMountPath( CONST WCHAR * aMountPath, std::wstring & aGuidPath );
//Retrieves the volume mount point where aFileOrDirPath is mounted
//aFileOrDirPath can be absolute or relative path
//aMountPath will be written as the path where aFileOrDirPath is mounted
//aMountPathLen is the length of aMountPath excluding terminating null
//Return TRUE if successful, FALSE otherwise
BOOL GetMountPathFromFileOrDirPath( CONST WCHAR * aFileOrDirPath, std::wstring & aMountPath );
//Retrieves the name of a mounted folder on the specified volume
//aMountedFolders will be the name of mounted folders on the specified volume
//aMountedFolders should have enough space([row][column] = [count of folders][MAX_PATH+1] WCHARs)
//Return how many folders mounted on the specific volume if successful, 0 otherwise
INT GetMountedFoldersFromGuidPath( CONST WCHAR * aVolumeGuidPath, WCHAR ** aMountedFolders );




//aDrivePath should be like "C:\" or "\\?\C:\"
//The type will be written to aVolumeType if it's not NULL, therefore it show have enough space
//Return the type in UINT defined in WinBase.h
UINT GetVolumeType( CONST WCHAR * aVolumePath, std::wstring & aVolumeType );
//aVolumePath should be like "C:\" or "\\?\C:\"
//aVolumeName will be written by drive's name
//aFs will be written by file system format, aFsLen is the buffer length excluding '\0', aFsFlag show the privilege
//aSerial show the serial of the drive OS assigned, not serial assigned by manufacture(use Win32_PhysicalMedia)
//aMaxFileLen is the max filename length this drive support, filename is the one between backslashes
//aFsFlag, aSerial, and aMaxFileLen are optional
//Return TRUE if successful, FALSE otherwise
BOOL GetVolumeInfo( CONST WCHAR * aVolumePath,
                    std::wstring & aVolumeName,
                    std::wstring & aFs,
                    DWORD * aFsFlag = NULL,
                    DWORD * aSerial = NULL,
                    DWORD * aMaxFileLen = NULL );

//Support from Windows Vista
//aVolumeName will be written by drive's name, aVolumeNameLen is the buffer length excluding '\0'
//aFs will be written by file system format, aFsLen is the buffer length excluding '\0', aFsFlag show the privilege
//aSerial show the serial of the drive OS assigned, not serial assigned by manufacture(use Win32_PhysicalMedia)
//aMaxFileLen is the max filename length this drive support, filename is the one between backslashes
//aFsFlag, aSerial, and aMaxFileLen are optional
//Return TRUE if successful, FALSE otherwise
//BOOL GetVolumeInfo( HANDLE hFile , WCHAR * aVolumeName , DWORD aVolumeNameLen ,
//                           WCHAR * aFs , DWORD aFsLen , DWORD & aFsFlag ,
//                           DWORD & aSerial , DWORD & aMaxFileLen );

//Get free space, total space for all users, or available free space for current user on the specific disk in bytes
//You can simply get one space information by setting others to NULL
//Return TRUE if successful, FALSE otherwise
BOOL GetVolumeSpace( CONST WCHAR * aVolumePath,
                     ULARGE_INTEGER * aFree = NULL,
                     ULARGE_INTEGER * aTotal = NULL,
                     ULARGE_INTEGER * aCurrAvailable = NULL );






//aVolumePath can be "C:\" or "C:\Windows\" that must end with backslash
//Return TRUE if successful, FALSE otherwise
BOOL SetVolumeName( WCHAR * aVolumePath, WCHAR * aNewName );

#ifdef __cplusplus
}
#endif

}    //End of namespace CWUtils
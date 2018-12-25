#pragma once

/*
 * Copyright (c) 2009-2015, ChienWei Hung <winestwinest@gmail.com>
 * CWUtils is published under the BSD-3-Clause license.
 *
 * CWUtils is a set of standalone APIs for developers to speed up their
 * programming. It should be very easy to port them to other projects or
 * learn how to implement things on different languages and platforms.
 *
 * The latest version can be found at https://github.com/winest/CWUtils
 */

#include "WinDef.h"
#include <algorithm>
#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace CWUtils {

#ifdef __cplusplus
extern "C" {
#endif

BOOL IsPathExist(CONST CHAR *aFullPath);

BOOL IsFileExist(CONST CHAR *aFullPath);

BOOL IsDirExist(CONST CHAR *aDirPath);

SIZE_T GetFileSize(CONST CHAR *aFilePath);

CONST SIZE_T FILE_BUF_SIZE = 4096;

CONST UINT32 FILE_OPEN_ATTR_NONE = 0x00000000; // Nothing
CONST UINT32 FILE_OPEN_ATTR_CREATE_IF_NOT_EXIST =
    0x00000001; // Open if exists, create if not exists
CONST UINT32 FILE_OPEN_ATTR_CREATE_ALWAYS = 0x00000002; // Always create new
                                                        // file
CONST UINT32 FILE_OPEN_ATTR_OPEN_EXISTING = 0x00000004; // Open if exists

CONST UINT32 FILE_OPEN_ATTR_BINARY = 0x00000008; // Open the raw file directly
CONST UINT32 FILE_OPEN_ATTR_MOVE_TO_END =
    0x00000010; // Move file pointer to the end of file
CONST UINT32 FILE_OPEN_ATTR_READ = 0x00000020;  // Open for read
CONST UINT32 FILE_OPEN_ATTR_WRITE = 0x00000040; // Open for write

class CFile {
public:
  CFile() : m_hFile(NULL) { m_strReadBuf.reserve(FILE_BUF_SIZE * 2); }
  virtual ~CFile() { this->Close(); }

public:
  BOOL Open(CONST CHAR *aPath, UINT32 aOpenAttr, std::string aLineSep);
  BOOL Open(CONST WCHAR *aPath, UINT32 aOpenAttr, std::string aLineSep);
  BOOL Write(CONST UCHAR *aData, SIZE_T aDataSize);
  BOOL WriteLine();
  BOOL WriteLine(CONST UCHAR *aData, SIZE_T aDataSize);
  BOOL Read(std::string &aData, SIZE_T aDataSize, BOOL aAppend = FALSE);
  BOOL ReadLine(std::string &aData, BOOL aAppend = FALSE);
  VOID Flush();
  VOID Close();

  FILE *GetFileHandle() { return m_hFile; }

protected:
  FILE *m_hFile;
  std::string m_strLineSep;
  std::string m_strReadBuf;
  size_t m_uReadPos;

  std::string m_strTmp;
};

class CCsv : public CFile {
public:
  CCsv() {}
  virtual ~CCsv() { this->Close(); }

public:
  BOOL WriteRow(CONST std::vector<std::string> &aColData, BOOL aAddQuote);
  BOOL ReadRow(std::vector<string> &aColData, BOOL aAppend = FALSE);
  BOOL ReadRow(std::vector<int> &aColData, BOOL aAppend = FALSE);
  BOOL ReadRow(std::vector<float> &aColData, BOOL aAppend = FALSE);
  BOOL ReadRow(std::vector<double> &aColData, BOOL aAppend = FALSE);
};

#ifdef __cplusplus
}
#endif

} // End of namespace CWUtils

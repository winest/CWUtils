##Coding convention:

- Naming rules mainly come from Symbian OS and some from MSDN. Symbian OS was used
  by Nokia in their feature phones. Since the OS was written in C++ and all applications
  running on Symbian need to provide source code to Nokia to ensure the quality and readibility,
  Nokia provides a great guideline about naming and is somehow adopted by CWUtils.

- To summarize convention:

  1. For all well known abbreviations, captialize the first letter and keep others in lower-case.
     For example, m_MACAddress -> m_MacAddress, IPV4Convert() -> Ipv4Convert()

  2. All constants are capatialized.
     For example,

     ```C++
     typedef enum _ScanRet
     {
         SCAN_RET_PASS = 0 ,
         SCAN_RET_BLOCK
     } ScanRet;
     ```
    
  3. All words are adjoined with mixed cases unless it's a constant.
     For example, do_something() -> DoSomething()

  4. All function arguments are started with "a" (stands for argument).
     This helps readers to know that variable comes from argument, and avoid the duplicate names used
     by local variables.
     For example,

     ```C++
        VOID DoSomthing( CHAR * aBuf , INT aIndex )
        {
            const CHAR * pBuf = (const CHAR *)aBuf;
        }
     ```

  5. Hungarian notation is used for local variables to avoid duplicate names
     For example,

     ```C++
        VOID SaveCurrIndex()
        {
            INT nCurrIndex = GetCurrIndex();
            CHAR szCurrIndex[10 + 1] = {};
            itoa( nCurrIndex , szCurrIndex , 10 );
            SaveToFile( szCurrIndex , strlen(szCurrIndex) );
        }
     ```

  6. For global variables, use "g_" at beginning.
     For class names, use "C" at beginning.
     For member data, use "m_" at beginning.
     For example,

     ```C++
        HEVENT g_hEvtStop;
        class CController()
        {
            private :
                BOOL m_bIsStarted;
        };
     ```

  7. Use specific size type and defined type so it can be easily ported to other platforms.
     Note that the size of built-in types varies on different platforms and some platforms
     don't support new standard types such int32_t in C++11.
     For example, int -> INT, int32_t -> INT32

  8. Brackets are always used in if/for/do/while/switch/case/default and inserted at a new line.
     For example,

     ```C++
        for ( size_t i = 11 ; i < 100 ; i++ )
        {
            if ( 10 == rand() % i )
            {
                break;
            }
        }
     ```

  9. Space is inserted before and after comma unless it's wrapped by other parentheses at the same line.
     For example,

     ```C++
        INT GetFinalDamage( CPlayer aAttacker , CPlayer aDefender )
        {
            INT nPhyDmg = GetPhyDmg( aAttacker , aDefender );
            INT nElementDmg = GetElementDmg( aAttacker , aDefender );
            return ( nPhyDmg * RandInt(1,10) + nElementDmg );
        }
     ```

  10. Keep constructors, destructors, and singleton function in header file for better negotiating with others.
      This helps those who use it as libraries and only has header files.

  11. Move all codes that may have dead lock issue from constructor to Init()/Reset() function
      and from destructor to UnInit()
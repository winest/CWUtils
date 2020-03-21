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
 * Provide stand-alone hash-related utilities for JScript developers.
 */

var CWUtils = CWUtils || {};




CWUtils.CByteHashBase = CWUtils.CByteHashBase || function()
{
    this.uInitHash;     //Shoube be overwritten by children
    this.uHash;         //Shoube be overwritten by children
    this.uHashSize;     //Shoube be overwritten by children

    this._ProcessBytes = function( aBuf , aBufSize ){};  //Should be overwritten by children
    this._Finalize = function(){};                       //Should be overwritten by children

    this.Init = function( aInitHash )
    {
        this.uHash = aInitHash || this.uInitHash || 0;
        this.uHashSize = 4;
        return this;
    }

    this.Update = function( aMsg )
    {
        var strMsg = aMsg;
        var uMsgBytes = aMsg.length;
        if ( typeof aMsg == "string" )
        {
            strMsg = unescape( encodeURIComponent(aMsg) );
            uMsgBytes = strMsg.length;
        }

        //Update the hash
        this._ProcessBytes( strMsg , uMsgBytes );

        //Chainable
        return this;
    }

    this.Finalize = function( aMsg )
    {
        //Final message update
        if ( aMsg )
        {
            this.Update( aMsg );
        }

        this._Finalize();

        //Return final computed hash by converting to HEX string
        return this._ToHexString( this.uHash , this.uHashSize );
    }

    this._ToHexString = function( aHash , aHashSize )
    {
        var hexChars = [];
        for ( var i = 0 ; i < aHashSize ; i++ )
        {
            var oneByte = ( aHash >>> (24 - (i % 4) * 8) ) & 0xff;
            hexChars.push( (oneByte >>> 4).toString(16) );
            hexChars.push( (oneByte & 0x0f).toString(16) );
        }

        return hexChars.join( "" );
    }
}
CWUtils.CCrc32 = CWUtils.CCrc32 || function()
{
    CWUtils.CByteHashBase.call( this );
    this.uInitHash = 0 ^ (-1);
    var hexTable = [ 0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3, 0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91, 0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7, 0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5, 0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B, 0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59, 0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F, 0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D, 0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433, 0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01, 0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457, 0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65, 0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB, 0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9, 0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F, 0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD, 0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683, 0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1, 0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7, 0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5, 0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B, 0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79, 0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F, 0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D, 0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713, 0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21, 0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777, 0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45, 0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB, 0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9, 0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF, 0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D ];

    this._ProcessBytes = function( aBuf , aBufSize )
    {
        for ( var i = 0 ; i < aBufSize ; i++ )
        {
            this.uHash  = ( this.uHash >>> 8 ) ^ hexTable[ (this.uHash ^ aBuf.charCodeAt(i)) & 0xFF ];
        }
    }

    this._Finalize = function()
    {
        this.uHash = ( this.uHash ^ -1 );
    }

    return this.Init( this.uInitHash );
}
CWUtils.Crc32 = CWUtils.Crc32 || function( aBuf )
{
    var crc32 = new CWUtils.CCrc32();
    return crc32.Finalize( aBuf );
}





CWUtils.CBlockHashBase = CWUtils.CBlockHashBase  || function()
{
    this.uBlockSize = 512 / 32;  //The number of DWORD this hasher operates on
    this.uMinBufSize = 0;        //The number of blocks that should be kept unprocessed in the buffer
    this.aryBuf;       //Saved in DWORD array
    this.uBufSize;     //Size of valid bytes in the DWORD array
    this.uTotalBytes;

    this.aryInitHash;   //Shoube be overwritten by children
    this.aryHash;       //Shoube be overwritten by children
    this.uHashSize;     //Shoube be overwritten by children

    this._ProcessBlock = function( aBuf , aOffset ){};   //Should be overwritten by children
    this._Finalize = function(){};                       //Should be overwritten by children

    this.Init = function( aInitHash )
    {
        this.aryBuf = [];
        this.uBufSize = 0;
        this.uTotalBytes = 0;

        this.aryHash = ( aInitHash || this.aryInitHash || [] ).slice( 0 );
        this.uHashSize = this.aryHash.length * 4;
        return this;
    }

    this.Update = function( aMsg )
    {
        //Append
        this._Append( aMsg );

        //Update the hash
        this._Process();

        //Chainable
        return this;
    }

    this.Finalize = function( aMsg )
    {
        //Final message update
        if ( aMsg )
        {
            this._Append( aMsg );
        }

        this._Finalize();

        //Return final computed hash by converting to HEX string
        return this._ToHexString( this.aryHash , this.uHashSize );
    }

    this._Append = function( aData )
    {
        //Convert string to DWORD array, else assume DWORD array already
        var aryNewData = aData;
        var uNewDataBytes = aData.length * 4;
        if ( typeof aData == "string" )
        {
            var strTmp = unescape( encodeURIComponent(aData) );
            uNewDataBytes = strTmp.length;

            //Convert
            aryNewData = [];
            for ( var i = 0 ; i < uNewDataBytes ; i++ )
            {
                aryNewData[i >>> 2] |= ( strTmp.charCodeAt(i) & 0xff ) << ( 24 - (i % 4) * 8 );
            }
        }



        //Removes insignificant bits
        this.aryBuf[this.uBufSize >>> 2] &= 0xffffffff << (32 - (this.uBufSize % 4) * 8);
        this.aryBuf.length = Math.ceil( this.uBufSize / 4 );

        //Concat
        if ( this.uBufSize % 4 )
        {
            //Copy one byte at a time
            for ( var i = 0 ; i < uNewDataBytes ; i++ )
            {
                var thatByte = ( aryNewData[i >>> 2] >>> (24 - (i % 4) * 8) ) & 0xff;
                this.aryBuf[(this.uBufSize + i) >>> 2] |= thatByte << ( 24 - ((this.uBufSize + i) % 4) * 8 );
            }
        }
        else if ( aryNewData.length > 0xffff )
        {
            //Copy one DWORD at a time
            for ( var i = 0 ; i < uNewDataBytes ; i += 4 )
            {
                this.aryBuf[(this.uBufSize + i) >>> 2] = aryNewData[i >>> 2];
            }
        }
        else
        {
            //Copy all DWORDs at once
            this.aryBuf.push.apply( this.aryBuf , aryNewData );
        }
        this.uBufSize += uNewDataBytes ;
        this.uTotalBytes += uNewDataBytes;
    }

    this._Process = function( aForceFlush )
    {
        //Count blocks ready
        var nBlocksReady = this.uBufSize / (this.uBlockSize * 4);
        if ( aForceFlush )
        {
            //Round up to include partial blocks
            nBlocksReady = Math.ceil( nBlocksReady );
        }
        else
        {
            //Round down to include only full blocks,
            //less the number of blocks that must remain in the buffer
            nBlocksReady = Math.max( (nBlocksReady | 0) - this.uMinBufSize , 0 );
        }

        var nDwordsReady = nBlocksReady * this.uBlockSize;               //DWORDs ready
        var nBytesReady = Math.min( nDwordsReady * 4 , this.uBufSize );  //Bytes ready

        //Process blocks
        if ( nDwordsReady )
        {
            for ( var offset = 0 ; offset < nDwordsReady ; offset += this.uBlockSize )
            {
                //Perform concrete-algorithm logic
                this._ProcessBlock( this.aryBuf , offset );
            }

            //Remove processed DWORDs
            this.aryBuf.splice( 0 , nDwordsReady );
            this.uBufSize -= nBytesReady;
        }
    }

    this._ToHexString = function( aDwordAry , aHashSize )
    {
        var hexChars = [];
        for ( var i = 0 ; i < aHashSize ; i++ )
        {
            var oneByte = ( aDwordAry[i >>> 2] >>> (24 - (i % 4) * 8) ) & 0xff;
            hexChars.push( (oneByte >>> 4).toString(16) );
            hexChars.push( (oneByte & 0x0f).toString(16) );
        }

        return hexChars.join( "" );
    }
}






CWUtils.CMd5 = CWUtils.CMd5 || function( aInitHash )
{
    CWUtils.CBlockHashBase.call( this );
    this.aryInitHash = aInitHash || [ 0x67452301 , 0xEFCDAB89 , 0x98BADCFE , 0x10325476 ];
    var T = [ 0xD76AA478 , 0xE8C7B756 , 0x242070DB , 0xC1BDCEEE , 0xF57C0FAF , 0x4787C62A , 0xA8304613 , 0xFD469501 , 0x698098D8 , 0x8B44F7AF , 0xFFFF5BB1 , 0x895CD7BE , 0x6B901122 , 0xFD987193 , 0xA679438E , 0x49B40821 , 0xF61E2562 , 0xC040B340 , 0x265E5A51 , 0xE9B6C7AA , 0xD62F105D , 0x02441453 , 0xD8A1E681 , 0xE7D3FBC8 , 0x21E1CDE6 , 0xC33707D6 , 0xF4D50D87 , 0x455A14ED , 0xA9E3E905 , 0xFCEFA3F8 , 0x676F02D9 , 0x8D2A4C8A , 0xFFFA3942 , 0x8771F681 , 0x6D9D6122 , 0xFDE5380C , 0xA4BEEA44 , 0x4BDECFA9 , 0xF6BB4B60 , 0xBEBFBC70 , 0x289B7EC6 , 0xEAA127FA , 0xD4EF3085 , 0x04881D05 , 0xD9D4D039 , 0xE6DB99E5 , 0x1FA27CF8 , 0xC4AC5665 , 0xF4292244 , 0x432AFF97 , 0xAB9423A7 , 0xFC93A039 , 0x655B59C3 , 0x8F0CCC92 , 0xFFEFF47D , 0x85845DD1 , 0x6FA87E4F , 0xFE2CE6E0 , 0xA3014314 , 0x4E0811A1 , 0xF7537E82 , 0xBD3AF235 , 0x2AD7D2BB , 0xEB86D391 ];

    function FF( a , b , c , d , x , s , t )
    {
        var n = a + ( (b & c) | (~b & d) ) + x + t;
        return (( n << s) | (n >>> (32 - s)) ) + b;
    }

    function GG( a , b , c , d , x , s , t )
    {
        var n = a + ( (b & d) | (c & ~d) ) + x + t;
        return ( (n << s) | (n >>> (32 - s)) ) + b;
    }

    function HH( a , b , c , d , x , s , t )
    {
        var n = a + ( b ^ c ^ d ) + x + t;
        return ( (n << s) | (n >>> (32 - s)) ) + b;
    }

    function II( a , b , c , d , x , s , t )
    {
        var n = a + ( c ^ (b | ~d) ) + x + t;
        return ( (n << s) | (n >>> (32 - s)) ) + b;
    }

    this._ProcessBlock = function( aBuf , aOffset )
    {
        //Swap endian
        for ( var i = 0 ; i < 16 ; i++ )
        {
            aBuf[aOffset + i] = ( (((aBuf[aOffset + i] << 8) | (aBuf[aOffset + i] >>> 24)) & 0x00ff00ff) |
                                  (((aBuf[aOffset + i] << 24) | (aBuf[aOffset + i] >>> 8)) & 0xff00ff00) );
        }

        //Working varialbes
        var a = this.aryHash[0];
        var b = this.aryHash[1];
        var c = this.aryHash[2];
        var d = this.aryHash[3];

        //Computation
        a = FF( a , b , c , d , aBuf[aOffset + 0] , 7 , T[0] );
        d = FF( d , a , b , c , aBuf[aOffset + 1] , 12 , T[1] );
        c = FF( c , d , a , b , aBuf[aOffset + 2] , 17 , T[2] );
        b = FF( b , c , d , a , aBuf[aOffset + 3] , 22 , T[3] );
        a = FF( a , b , c , d , aBuf[aOffset + 4] , 7 , T[4] );
        d = FF( d , a , b , c , aBuf[aOffset + 5] , 12 , T[5] );
        c = FF( c , d , a , b , aBuf[aOffset + 6] , 17 , T[6] );
        b = FF( b , c , d , a , aBuf[aOffset + 7] , 22 , T[7] );
        a = FF( a , b , c , d , aBuf[aOffset + 8] , 7 , T[8] );
        d = FF( d , a , b , c , aBuf[aOffset + 9] , 12 , T[9] );
        c = FF( c , d , a , b , aBuf[aOffset + 10] , 17 , T[10] );
        b = FF( b , c , d , a , aBuf[aOffset + 11] , 22 , T[11] );
        a = FF( a , b , c , d , aBuf[aOffset + 12] , 7 , T[12] );
        d = FF( d , a , b , c , aBuf[aOffset + 13] , 12 , T[13] );
        c = FF( c , d , a , b , aBuf[aOffset + 14] , 17 , T[14] );
        b = FF( b , c , d , a , aBuf[aOffset + 15] , 22 , T[15] );

        a = GG( a , b , c , d , aBuf[aOffset + 1] , 5 , T[16] );
        d = GG( d , a , b , c , aBuf[aOffset + 6] , 9 , T[17] );
        c = GG( c , d , a , b , aBuf[aOffset + 11] , 14 , T[18] );
        b = GG( b , c , d , a , aBuf[aOffset + 0] , 20 , T[19] );
        a = GG( a , b , c , d , aBuf[aOffset + 5] , 5 , T[20] );
        d = GG( d , a , b , c , aBuf[aOffset + 10] , 9 , T[21] );
        c = GG( c , d , a , b , aBuf[aOffset + 15] , 14 , T[22] );
        b = GG( b , c , d , a , aBuf[aOffset + 4] , 20 , T[23] );
        a = GG( a , b , c , d , aBuf[aOffset + 9] , 5 , T[24] );
        d = GG( d , a , b , c , aBuf[aOffset + 14] , 9 , T[25] );
        c = GG( c , d , a , b , aBuf[aOffset + 3] , 14 , T[26] );
        b = GG( b , c , d , a , aBuf[aOffset + 8] , 20 , T[27] );
        a = GG( a , b , c , d , aBuf[aOffset + 13] , 5 , T[28] );
        d = GG( d , a , b , c , aBuf[aOffset + 2] , 9 , T[29] );
        c = GG( c , d , a , b , aBuf[aOffset + 7] , 14 , T[30] );
        b = GG( b , c , d , a , aBuf[aOffset + 12] , 20 , T[31] );

        a = HH( a , b , c , d , aBuf[aOffset + 5] , 4 , T[32] );
        d = HH( d , a , b , c , aBuf[aOffset + 8] , 11 , T[33] );
        c = HH( c , d , a , b , aBuf[aOffset + 11] , 16 , T[34] );
        b = HH( b , c , d , a , aBuf[aOffset + 14] , 23 , T[35] );
        a = HH( a , b , c , d , aBuf[aOffset + 1] , 4 , T[36] );
        d = HH( d , a , b , c , aBuf[aOffset + 4] , 11 , T[37] );
        c = HH( c , d , a , b , aBuf[aOffset + 7] , 16 , T[38] );
        b = HH( b , c , d , a , aBuf[aOffset + 10] , 23 , T[39] );
        a = HH( a , b , c , d , aBuf[aOffset + 13] , 4 , T[40] );
        d = HH( d , a , b , c , aBuf[aOffset + 0] , 11 , T[41] );
        c = HH( c , d , a , b , aBuf[aOffset + 3] , 16 , T[42] );
        b = HH( b , c , d , a , aBuf[aOffset + 6] , 23 , T[43] );
        a = HH( a , b , c , d , aBuf[aOffset + 9] , 4 , T[44] );
        d = HH( d , a , b , c , aBuf[aOffset + 12] , 11 , T[45] );
        c = HH( c , d , a , b , aBuf[aOffset + 15] , 16 , T[46] );
        b = HH( b , c , d , a , aBuf[aOffset + 2] , 23 , T[47] );

        a = II( a , b , c , d , aBuf[aOffset + 0] , 6 , T[48] );
        d = II( d , a , b , c , aBuf[aOffset + 7] , 10 , T[49] );
        c = II( c , d , a , b , aBuf[aOffset + 14] , 15 , T[50] );
        b = II( b , c , d , a , aBuf[aOffset + 5] , 21 , T[51] );
        a = II( a , b , c , d , aBuf[aOffset + 12] , 6 , T[52] );
        d = II( d , a , b , c , aBuf[aOffset + 3] , 10 , T[53] );
        c = II( c , d , a , b , aBuf[aOffset + 10] , 15 , T[54] );
        b = II( b , c , d , a , aBuf[aOffset + 1] , 21 , T[55] );
        a = II( a , b , c , d , aBuf[aOffset + 8] , 6 , T[56] );
        d = II( d , a , b , c , aBuf[aOffset + 15] , 10 , T[57] );
        c = II( c , d , a , b , aBuf[aOffset + 6] , 15 , T[58] );
        b = II( b , c , d , a , aBuf[aOffset + 13] , 21 , T[59] );
        a = II( a , b , c , d , aBuf[aOffset + 4] , 6 , T[60] );
        d = II( d , a , b , c , aBuf[aOffset + 11] , 10 , T[61] );
        c = II( c , d , a , b , aBuf[aOffset + 2] , 15 , T[62] );
        b = II( b , c , d , a , aBuf[aOffset + 9] , 21 , T[63] );

        //Intermediate hash value
        this.aryHash[0] = ( this.aryHash[0] + a ) | 0;
        this.aryHash[1] = ( this.aryHash[1] + b ) | 0;
        this.aryHash[2] = ( this.aryHash[2] + c ) | 0;
        this.aryHash[3] = ( this.aryHash[3] + d ) | 0;
    }



    this._Finalize =  function()
    {
        var nBitsTotal = this.uTotalBytes * 8;
        var nBitsLeft = this.uBufSize * 8;

        //Add padding
        this.aryBuf[nBitsLeft >>> 5] |= 0x80 << (24 - nBitsLeft % 32);

        var nBitsTotalH = Math.floor(nBitsTotal / 0x100000000);
        var nBitsTotalL = nBitsTotal;
        this.aryBuf[(((nBitsLeft + 64) >>> 9) << 4) + 15] = (
            (((nBitsTotalH << 8) | (nBitsTotalH >>> 24)) & 0x00ff00ff) |
            (((nBitsTotalH << 24) | (nBitsTotalH >>> 8)) & 0xff00ff00)
        );
        this.aryBuf[(((nBitsLeft + 64) >>> 9) << 4) + 14] = (
            (((nBitsTotalL << 8) | (nBitsTotalL >>> 24)) & 0x00ff00ff) |
            (((nBitsTotalL << 24) | (nBitsTotalL >>> 8)) & 0xff00ff00)
        );

        this.uBufSize = (this.aryBuf.length + 1) * 4;

        //Hash final blocks
        this._Process();

        //Swap endian
        for ( var i = 0 ; i < 4 ; i++ )
        {
            this.aryHash[i] = ( ((this.aryHash[i] << 8) | (this.aryHash[i] >>> 24)) & 0x00ff00ff ) |
                              ( ((this.aryHash[i] << 24) | (this.aryHash[i] >>> 8)) & 0xff00ff00 );
        }
    }

    return this.Init( this.aryInitHash );
}
CWUtils.Md5 = CWUtils.Md5 || function( aBuf )
{
    var md5 = new CWUtils.CMd5();
    return md5.Finalize( aBuf );
}









CWUtils.CSha1 = CWUtils.CSha1 || function( aInitHash )
{
    CWUtils.CBlockHashBase.call( this );
    this.aryInitHash = aInitHash || [ 0x67452301 , 0xEFCDAB89 , 0x98BADCFE , 0x10325476 , 0xC3D2E1F0 ];
    var W = [];

    this._ProcessBlock = function( aBuf , aOffset )
    {
        //Working variables
        var a = this.aryHash[0];
        var b = this.aryHash[1];
        var c = this.aryHash[2];
        var d = this.aryHash[3];
        var e = this.aryHash[4];

        //Computation
        for ( var i = 0 ; i < 80 ; i++ )
        {
            if ( i < 16 )
            {
                W[i] = aBuf[aOffset + i] | 0;
            }
            else
            {
                var n = W[i - 3] ^ W[i - 8] ^ W[i - 14] ^ W[i - 16];
                W[i] = (n << 1) | (n >>> 31);
            }

            var t = ( (a << 5) | (a >>> 27) ) + e + W[i];
            if ( i < 20 )
            {
                t += ( (b & c) | (~b & d) ) + 0x5A827999;
            }
            else if ( i < 40 )
            {
                t += ( b ^ c ^ d ) + 0x6ED9EBA1;
            }
            else if ( i < 60 )
            {
                t += ( (b & c) | (b & d) | (c & d) ) - 0x70E44324;
            }
            else /* if (i < 80) */
            {
                t += ( b ^ c ^ d ) - 0x359D3E2A;
            }

            e = d;
            d = c;
            c = ( b << 30 ) | ( b >>> 2 );
            b = a;
            a = t;
        }

        //Intermediate hash value
        this.aryHash[0] = (this.aryHash[0] + a) | 0;
        this.aryHash[1] = (this.aryHash[1] + b) | 0;
        this.aryHash[2] = (this.aryHash[2] + c) | 0;
        this.aryHash[3] = (this.aryHash[3] + d) | 0;
        this.aryHash[4] = (this.aryHash[4] + e) | 0;
    }


    this._Finalize = function()
    {
        //Perform concrete-hasher logic
        var nBitsTotal = this.uTotalBytes * 8;
        var nBitsLeft = this.uBufSize * 8;

        //Add padding
        this.aryBuf[nBitsLeft >>> 5] |= 0x80 << ( 24 - nBitsLeft % 32 );
        this.aryBuf[(((nBitsLeft + 64) >>> 9) << 4) + 14] = Math.floor( nBitsTotal / 0x100000000 );
        this.aryBuf[(((nBitsLeft + 64) >>> 9) << 4) + 15] = nBitsTotal;
        this.uBufSize = this.aryBuf.length * 4;

        //Hash final blocks
        this._Process();
    }

    return this.Init( this.aryInitHash );
}
CWUtils.Sha1 = CWUtils.Sha1 || function( aBuf )
{
    var sha1 = new CWUtils.CSha1();
    return sha1.Finalize( aBuf );
}


CWUtils.CSha256 = CWUtils.CSha256 || function( aInitHash )
{
    CWUtils.CBlockHashBase.call( this );
    this.aryInitHash = aInitHash || [ 0x6A09E667 , 0xBB67AE85 , 0x3C6EF372 , 0xA54FF53A , 0x510E527F , 0x9B05688C , 0x1F83D9AB , 0x5BE0CD19 ];
    var W = [];
    var K = [ 0x428A2F98 , 0x71374491 , 0xB5C0FBCF , 0xE9B5DBA5 , 0x3956C25B , 0x59F111F1 , 0x923F82A4 , 0xAB1C5ED5 , 0xD807AA98 , 0x12835B01 , 0x243185BE , 0x550C7DC3 , 0x72BE5D74 , 0x80DEB1FE , 0x9BDC06A7 , 0xC19BF174 , 0xE49B69C1 , 0xEFBE4786 , 0xFC19DC6 , 0x240CA1CC , 0x2DE92C6F , 0x4A7484AA , 0x5CB0A9DC , 0x76F988DA , 0x983E5152 , 0xA831C66D , 0xB00327C8 , 0xBF597FC7 , 0xC6E00BF3 , 0xD5A79147 , 0x6CA6351 , 0x14292967 , 0x27B70A85 , 0x2E1B2138 , 0x4D2C6DFC , 0x53380D13 , 0x650A7354 , 0x766A0ABB , 0x81C2C92E , 0x92722C85 , 0xA2BFE8A1 , 0xA81A664B , 0xC24B8B70 , 0xC76C51A3 , 0xD192E819 , 0xD6990624 , 0xF40E3585 , 0x106AA070 , 0x19A4C116 , 0x1E376C08 , 0x2748774C , 0x34B0BCB5 , 0x391C0CB3 , 0x4ED8AA4A , 0x5B9CCA4F , 0x682E6FF3 , 0x748F82EE , 0x78A5636F , 0x84C87814 , 0x8CC70208 , 0x90BEFFFA , 0xA4506CEB , 0xBEF9A3F7 , 0xC67178F2 ];

    this._ProcessBlock = function( aBuf , aOffset )
    {
        // Working variables
        var a = this.aryHash[0];
        var b = this.aryHash[1];
        var c = this.aryHash[2];
        var d = this.aryHash[3];
        var e = this.aryHash[4];
        var f = this.aryHash[5];
        var g = this.aryHash[6];
        var h = this.aryHash[7];

        // Computation
        for ( var i = 0 ; i < 64 ; i++ )
        {
            if ( i < 16 )
            {
                W[i] = aBuf[aOffset + i] | 0;
            }
            else
            {
                var gamma0x = W[i - 15];
                var gamma0 = ( (gamma0x << 25) | (gamma0x >>> 7) ) ^
                             ( (gamma0x << 14) | (gamma0x >>> 18) ) ^
                             ( gamma0x >>> 3 );

                var gamma1x = W[i - 2];
                var gamma1 = ( (gamma1x << 15) | (gamma1x >>> 17) ) ^
                             ( (gamma1x << 13) | (gamma1x >>> 19) ) ^
                             ( gamma1x >>> 10 );

                W[i] = gamma0 + W[i - 7] + gamma1 + W[i - 16];
            }

            var ch = ( e & f ) ^ ( ~e & g );
            var maj = ( a & b ) ^ ( a & c ) ^ ( b & c );

            var sigma0 = ( (a << 30) | (a >>> 2) ) ^ ( (a << 19) | (a >>> 13) ) ^ ( (a << 10) | (a >>> 22) );
            var sigma1 = ( (e << 26) | (e >>> 6))  ^ ( (e << 21) | (e >>> 11) ) ^ ( (e << 7) | (e >>> 25) );

            var t1 = h + sigma1 + ch + K[i] + W[i];
            var t2 = sigma0 + maj;

            h = g;
            g = f;
            f = e;
            e = ( d + t1 ) | 0;
            d = c;
            c = b;
            b = a;
            a = ( t1 + t2 ) | 0;
        }

        //Intermediate hash value
        this.aryHash[0] = ( this.aryHash[0] + a ) | 0;
        this.aryHash[1] = ( this.aryHash[1] + b ) | 0;
        this.aryHash[2] = ( this.aryHash[2] + c ) | 0;
        this.aryHash[3] = ( this.aryHash[3] + d ) | 0;
        this.aryHash[4] = ( this.aryHash[4] + e ) | 0;
        this.aryHash[5] = ( this.aryHash[5] + f ) | 0;
        this.aryHash[6] = ( this.aryHash[6] + g ) | 0;
        this.aryHash[7] = ( this.aryHash[7] + h ) | 0;
    }

    this._Finalize = function()
    {
        //Perform concrete-hasher logic
        var nBitsTotal = this.uTotalBytes * 8;
        var nBitsLeft = this.uBufSize * 8;

        //Add padding
        this.aryBuf[nBitsLeft >>> 5] |= 0x80 << ( 24 - nBitsLeft % 32 );
        this.aryBuf[(((nBitsLeft + 64) >>> 9) << 4) + 14] = Math.floor( nBitsTotal / 0x100000000 );
        this.aryBuf[(((nBitsLeft + 64) >>> 9) << 4) + 15] = nBitsTotal;
        this.uBufSize = this.aryBuf.length * 4;

        //Hash final blocks
        this._Process();
    }

    return this.Init( this.aryInitHash );
}
CWUtils.Sha256 = CWUtils.Sha256 || function( aBuf )
{
    var sha256 = new CWUtils.CSha256();
    return sha256.Finalize( aBuf );
}



CWUtils.CSha224 = CWUtils.CSha224 || function( aInitHash )
{
    var pThis = CWUtils.CSha256.call( this , aInitHash || [ 0xC1059ED8 , 0x367CD507 , 0x3070DD17 , 0xF70E5939 , 0xFFC00B31 , 0x68581511 , 0x64F98FA7 , 0xBEFA4FA4 ] );
    var pfnSha256Finalize = pThis._Finalize;
    this._Finalize = function()
    {
        pfnSha256Finalize.call( this );
        this.uHashSize -= 4;
    }

    return pThis;
}
CWUtils.Sha224 = CWUtils.Sha224 || function( aBuf )
{
    var sha224 = new CWUtils.CSha224();
    return sha224.Finalize( aBuf );
}




CWUtils.CSha512 = CWUtils.CSha512 || function( aInitHash )
{
    CWUtils.CBlockHashBase.call( this );
    this.uBlockSize = 1024 / 32;
    this.aryInitHash = aInitHash || [ 0x6A09E667 , 0xF3BCC908 , 0xBB67AE85, 0x84CAA73B , 0x3C6EF372 , 0xFE94F82B , 0xA54FF53A , 0x5F1D36F1 , 0x510E527F , 0xADE682D1 , 0x9B05688C, 0x2B3E6C1F , 0x1F83D9AB , 0xFB41BD6B , 0x5BE0CD19, 0x137E2179 ];
    var K = [ 0x428A2F98 , 0xD728AE22 , 0x71374491 , 0x23EF65CD , 0xB5C0FBCF , 0xEC4D3B2F , 0xE9B5DBA5 , 0x8189DBBC , 0x3956C25B , 0xF348B538 , 0x59F111F1 , 0xB605D019 , 0x923F82A4 , 0xAF194F9B , 0xAB1C5ED5 , 0xDA6D8118 , 0xD807AA98 , 0xA3030242 , 0x12835B01 , 0x45706FBE , 0x243185BE , 0x4EE4B28C , 0x550C7DC3 , 0xD5FFB4E2 , 0x72BE5D74 , 0xF27B896F , 0x80DEB1FE , 0x3B1696B1 , 0x9BDC06A7 , 0x25C71235 , 0xC19BF174 , 0xCF692694 , 0xE49B69C1 , 0x9EF14AD2 , 0xEFBE4786 , 0x384F25E3 , 0x0FC19DC6 , 0x8B8CD5B5 , 0x240CA1CC , 0x77AC9C65 , 0x2DE92C6F , 0x592B0275 , 0x4A7484AA , 0x6EA6E483 , 0x5CB0A9DC , 0xBD41FBD4 , 0x76F988DA , 0x831153B5 , 0x983E5152 , 0xEE66DFAB , 0xA831C66D , 0x2DB43210 , 0xB00327C8 , 0x98FB213F , 0xBF597FC7 , 0xBEEF0EE4 , 0xC6E00BF3 , 0x3DA88FC2 , 0xD5A79147 , 0x930AA725 , 0x06CA6351 , 0xE003826F , 0x14292967 , 0x0A0E6E70 , 0x27B70A85 , 0x46D22FFC , 0x2E1B2138 , 0x5C26C926 , 0x4D2C6DFC , 0x5AC42AED , 0x53380D13 , 0x9D95B3DF , 0x650A7354 , 0x8BAF63DE , 0x766A0ABB , 0x3C77B2A8 , 0x81C2C92E , 0x47EDAEE6 , 0x92722C85 , 0x1482353B , 0xA2BFE8A1 , 0x4CF10364 , 0xA81A664B , 0xBC423001 , 0xC24B8B70 , 0xD0F89791 , 0xC76C51A3 , 0x0654BE30 , 0xD192E819 , 0xD6EF5218 , 0xD6990624 , 0x5565A910 , 0xF40E3585 , 0x5771202A , 0x106AA070 , 0x32BBD1B8 , 0x19A4C116 , 0xB8D2D0C8 , 0x1E376C08 , 0x5141AB53 , 0x2748774C , 0xDF8EEB99 , 0x34B0BCB5 , 0xE19B48A8 , 0x391C0CB3 , 0xC5C95A63 , 0x4ED8AA4A , 0xE3418ACB , 0x5B9CCA4F , 0x7763E373 , 0x682E6FF3 , 0xD6B2B8A3 , 0x748F82EE , 0x5DEFB2FC , 0x78A5636F , 0x43172F60 , 0x84C87814 , 0xA1F0AB72 , 0x8CC70208 , 0x1A6439EC , 0x90BEFFFA , 0x23631E28 , 0xA4506CEB , 0xDE82BDE9 , 0xBEF9A3F7 , 0xB2C67915 , 0xC67178F2 , 0xE372532B , 0xCA273ECE , 0xEA26619C , 0xD186B8C7 , 0x21C0C207 , 0xEADA7DD6 , 0xCDE0EB1E , 0xF57D4F7F , 0xEE6ED178 , 0x06F067AA , 0x72176FBA , 0x0A637DC5 , 0xA2C898A6 , 0x113F9804 , 0xBEF90DAE , 0x1B710B35 , 0x131C471B , 0x28DB77F5 , 0x23047D84 , 0x32CAAB7B , 0x40C72493 , 0x3C9EBE0A , 0x15C9BEBC , 0x431D67C4 , 0x9C100D4C , 0x4CC5D4BE , 0xCB3E42B6 , 0x597F299C , 0xFC657E2A , 0x5FCB6FAB , 0x3AD6FAEC , 0x6C44198C , 0x4A475817 ];
    var W = [];

    this._ProcessBlock = function( aBuf , aOffset )
    {
        var H0h = this.aryHash[0];
        var H0l = this.aryHash[1];
        var H1h = this.aryHash[2];
        var H1l = this.aryHash[3];
        var H2h = this.aryHash[4];
        var H2l = this.aryHash[5];
        var H3h = this.aryHash[6];
        var H3l = this.aryHash[7];
        var H4h = this.aryHash[8];
        var H4l = this.aryHash[9];
        var H5h = this.aryHash[10];
        var H5l = this.aryHash[11];
        var H6h = this.aryHash[12];
        var H6l = this.aryHash[13];
        var H7h = this.aryHash[14];
        var H7l = this.aryHash[15];

        // Working variables
        var ah = H0h;
        var al = H0l;
        var bh = H1h;
        var bl = H1l;
        var ch = H2h;
        var cl = H2l;
        var dh = H3h;
        var dl = H3l;
        var eh = H4h;
        var el = H4l;
        var fh = H5h;
        var fl = H5l;
        var gh = H6h;
        var gl = H6l;
        var hh = H7h;
        var hl = H7l;

        // Rounds
        for (var i = 0; i < 80; i++)
        {
            // Extend message
            if (i < 16)
            {
                var Wih = W[i*2] = aBuf[aOffset + i * 2] | 0;
                var Wil = W[i*2 + 1] = aBuf[aOffset + i * 2 + 1] | 0;
            }
            else
            {
                // Gamma0
                var gamma0xh = W[(i - 15)*2];
                var gamma0xl = W[(i - 15)*2 + 1];
                var gamma0h = ((gamma0xh >>> 1) | (gamma0xl << 31)) ^ ((gamma0xh >>> 8) | (gamma0xl << 24)) ^ (gamma0xh >>> 7);
                var gamma0l = ((gamma0xl >>> 1) | (gamma0xh << 31)) ^ ((gamma0xl >>> 8) | (gamma0xh << 24)) ^ ((gamma0xl >>> 7) | (gamma0xh << 25));

                // Gamma1
                var gamma1xh = W[(i - 2)*2];
                var gamma1xl = W[(i - 2)*2 + 1];
                var gamma1h = ((gamma1xh >>> 19) | (gamma1xl << 13)) ^ ((gamma1xh << 3) | (gamma1xl >>> 29)) ^ (gamma1xh >>> 6);
                var gamma1l = ((gamma1xl >>> 19) | (gamma1xh << 13)) ^ ((gamma1xl << 3) | (gamma1xh >>> 29)) ^ ((gamma1xl >>> 6) | (gamma1xh << 26));

                // W[i] = gamma0 + W[i - 7] + gamma1 + W[i - 16]
                var Wi7h = W[(i - 7)*2];
                var Wi7l = W[(i - 7)*2 + 1];

                var Wi16h = W[(i - 16)*2];
                var Wi16l = W[(i - 16)*2 + 1];

                var Wil = gamma0l + Wi7l;
                var Wih = gamma0h + Wi7h + ((Wil >>> 0) < (gamma0l >>> 0) ? 1 : 0);
                var Wil = Wil + gamma1l;
                var Wih = Wih + gamma1h + ((Wil >>> 0) < (gamma1l >>> 0) ? 1 : 0);
                var Wil = Wil + Wi16l;
                var Wih = Wih + Wi16h + ((Wil >>> 0) < (Wi16l >>> 0) ? 1 : 0);

                W[i*2] = Wih;
                W[i*2+1] = Wil;
            }

            var chh = (eh & fh) ^ (~eh & gh);
            var chl = (el & fl) ^ (~el & gl);
            var majh = (ah & bh) ^ (ah & ch) ^ (bh & ch);
            var majl = (al & bl) ^ (al & cl) ^ (bl & cl);

            var sigma0h = ((ah >>> 28) | (al << 4)) ^ ((ah << 30) | (al >>> 2)) ^ ((ah << 25) | (al >>> 7));
            var sigma0l = ((al >>> 28) | (ah << 4)) ^ ((al << 30) | (ah >>> 2)) ^ ((al << 25) | (ah >>> 7));
            var sigma1h = ((eh >>> 14) | (el << 18)) ^ ((eh >>> 18) | (el << 14)) ^ ((eh << 23) | (el >>> 9));
            var sigma1l = ((el >>> 14) | (eh << 18)) ^ ((el >>> 18) | (eh << 14)) ^ ((el << 23) | (eh >>> 9));

            // t1 = h + sigma1 + ch + K[i] + W[i]
            var Kih = K[i*2];
            var Kil = K[i*2 + 1];

            var t1l = hl + sigma1l;
            var t1h = hh + sigma1h + ((t1l >>> 0) < (hl >>> 0) ? 1 : 0);
            var t1l = t1l + chl;
            var t1h = t1h + chh + ((t1l >>> 0) < (chl >>> 0) ? 1 : 0);
            var t1l = t1l + Kil;
            var t1h = t1h + Kih + ((t1l >>> 0) < (Kil >>> 0) ? 1 : 0);
            var t1l = t1l + Wil;
            var t1h = t1h + Wih + ((t1l >>> 0) < (Wil >>> 0) ? 1 : 0);

            // t2 = sigma0 + maj
            var t2l = sigma0l + majl;
            var t2h = sigma0h + majh + ((t2l >>> 0) < (sigma0l >>> 0) ? 1 : 0);

            // Update working variables
            hh = gh;
            hl = gl;
            gh = fh;
            gl = fl;
            fh = eh;
            fl = el;
            el = (dl + t1l) | 0;
            eh = (dh + t1h + ((el >>> 0) < (dl >>> 0) ? 1 : 0)) | 0;
            dh = ch;
            dl = cl;
            ch = bh;
            cl = bl;
            bh = ah;
            bl = al;
            al = (t1l + t2l) | 0;
            ah = (t1h + t2h + ((al >>> 0) < (t1l >>> 0) ? 1 : 0)) | 0;
        }

        // Intermediate hash value
        H0l = this.aryHash[1] = (H0l + al);
        this.aryHash[0] = (H0h + ah + ((H0l >>> 0) < (al >>> 0) ? 1 : 0));
        H1l = this.aryHash[3] = (H1l + bl);
        this.aryHash[2] = (H1h + bh + ((H1l >>> 0) < (bl >>> 0) ? 1 : 0));
        H2l = this.aryHash[5] = (H2l + cl);
        this.aryHash[4] = (H2h + ch + ((H2l >>> 0) < (cl >>> 0) ? 1 : 0));
        H3l = this.aryHash[7] = (H3l + dl);
        this.aryHash[6] = (H3h + dh + ((H3l >>> 0) < (dl >>> 0) ? 1 : 0));
        H4l = this.aryHash[9] = (H4l + el);
        this.aryHash[8] = (H4h + eh + ((H4l >>> 0) < (el >>> 0) ? 1 : 0));
        H5l = this.aryHash[11] = (H5l + fl);
        this.aryHash[10] = (H5h + fh + ((H5l >>> 0) < (fl >>> 0) ? 1 : 0));
        H6l = this.aryHash[13] = (H6l + gl);
        this.aryHash[12] = (H6h + gh + ((H6l >>> 0) < (gl >>> 0) ? 1 : 0));
        H7l = this.aryHash[15] = (H7l + hl);
        this.aryHash[14] = (H7h + hh + ((H7l >>> 0) < (hl >>> 0) ? 1 : 0));
    }

    this._Finalize = function()
    {
        var nBitsTotal = this.uTotalBytes * 8;
        var nBitsLeft = this.uBufSize * 8;

        //Add padding
        this.aryBuf[nBitsLeft >>> 5] |= 0x80 << (24 - nBitsLeft % 32);
        this.aryBuf[(((nBitsLeft + 128) >>> 10) << 5) + 30] = Math.floor(nBitsTotal / 0x100000000);
        this.aryBuf[(((nBitsLeft + 128) >>> 10) << 5) + 31] = nBitsTotal;
        this.uBufSize = this.aryBuf.length * 4;

        //Hash final blocks
        this._Process();
    }

    return this.Init( this.aryInitHash );
}
CWUtils.Sha512 = CWUtils.Sha512 || function( aBuf )
{
    var sha512 = new CWUtils.CSha512();
    return sha512.Finalize( aBuf );
}

CWUtils.CSha384 = CWUtils.CSha384 || function( aInitHash )
{
    var pThis = CWUtils.CSha512.call( this , aInitHash || [ 0xCBBB9D5D , 0xC1059ED8 , 0x629A292A , 0x367CD507 , 0x9159015A , 0x3070DD17 , 0x152FECD8 , 0xF70E5939 , 0x67332667 , 0xFFC00B31 , 0x8EB44A87 , 0x68581511 , 0xDB0C2E0D , 0x64F98FA7 , 0x47B5481D , 0xBEFA4FA4 ] );
    var pfnSha512Finalize = pThis._Finalize;
    this._Finalize = function()
    {
        pfnSha512Finalize.call( this );
        this.uHashSize -= 16;
    }

    return pThis;
}
CWUtils.Sha384 = CWUtils.Sha384 || function( aBuf )
{
    var sha384 = new CWUtils.CSha384();
    return sha384.Finalize( aBuf );
}
/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library * 
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP.h                       * 
*    date added: 01-31-2005                      *
* date modified: 12-01-2006                      *
*       version: 1.06                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       * 
*                                                *
* description: Main include file                 *
*                                                *
*************************************************/
#ifndef __EASY_BITMAP_H__
#define __EASY_BITMAP_H__
#include <iostream>
#include <cmath>
#include <cctype>
#include <cstring>
#include <stdio.h>
#include <cdtypes.h>
#include <cairomm/surface.h>
#ifndef _DefaultXPelsPerMeter_
#define _DefaultXPelsPerMeter_
#define DefaultXPelsPerMeter 3780
#define DefaultYPelsPerMeter 3780
// set to a default of 96 dpi
#endif


#ifndef _EasyBMP_Version_
#define _EasyBMP_Version_ 1.06
#define _EasyBMP_Version_Integer_ 106
#define _EasyBMP_Version_String_ "1.06"
#endif

#ifndef _EasyBMPwarnings_
#define _EasyBMPwarnings_
#endif

using namespace Cairo;

typedef struct RGBAColor {
    BYTE Blue;
    BYTE Green;
    BYTE Red;
    BYTE Alpha;
} RGBAColor;

class Bitmap{
private:
    int BitDepth;
    int Width;
    int Height;
    UINT* Pixels;
    RGBAColor* Colors;//Color Table
    int XPelsPerMeter;
    int YPelsPerMeter;

    BYTE* MetaData1;
    int SizeOfMetaData1;
    BYTE* MetaData2;
    int SizeOfMetaData2;

    RefPtr<ImageSurface>image;
    bool Read32bitRow( BYTE* Buffer, int BufferSize, int Row );
    bool Read24bitRow( BYTE* Buffer, int BufferSize, int Row );
    bool Read8bitRow(  BYTE* Buffer, int BufferSize, int Row );
    bool Read4bitRow(  BYTE* Buffer, int BufferSize, int Row );
    bool Read1bitRow(  BYTE* Buffer, int BufferSize, int Row );

    bool Write32bitRow( BYTE* Buffer, int BufferSize, int Row );
    bool Write24bitRow( BYTE* Buffer, int BufferSize, int Row );
    bool Write8bitRow(  BYTE* Buffer, int BufferSize, int Row );
    bool Write4bitRow(  BYTE* Buffer, int BufferSize, int Row );
    bool Write1bitRow(  BYTE* Buffer, int BufferSize, int Row );

    BYTE FindClosestColor( RGBAColor& input );
    bool SafeRead( char* buffer, int size, int number,std::istream&);
public:

    int TellBitDepth( void );
    int TellWidth( void );
    int TellHeight( void );
    int TellNumberOfColors( void );
    void SetDPI( int HorizontalDPI, int VerticalDPI );
    int TellVerticalDPI( void );
    int TellHorizontalDPI( void );

    Bitmap();
    Bitmap( Bitmap& Input );
    ~Bitmap();
    
    UINT GetPixel( int i, int j ) const;
    bool SetPixel( int i, int j,UINT NewPixel );

    bool CreateStandardColorTable( void );

    bool SetSize( int NewWidth, int NewHeight );
    bool SetBitDepth( int NewDepth );
    bool WriteToStream(std::ostream&);
    bool ReadFromStream(std::istream&is);

    RGBAColor GetColor( int ColorNumber );
    UINT RGBA2Pixel(int coloridx);
    bool SetColor( int ColorNumber,RGBAColor NewColor );
    void CopyPixels(void*buff,int stride=-1);
    operator RefPtr<ImageSurface>();
};


#endif

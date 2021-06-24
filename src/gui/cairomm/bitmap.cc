/*************************************************
*                                                *
*  EasyBMP Cross-Platform Windows Bitmap Library *
*                                                *
*  Author: Paul Macklin                          *
*   email: macklin01@users.sourceforge.net       *
* support: http://easybmp.sourceforge.net        *
*                                                *
*          file: EasyBMP.cpp                     *
*    date added: 03-31-2006                      *
* date modified: 12-01-2006                      *
*       version: 1.06                            *
*                                                *
*   License: BSD (revised/modified)              *
* Copyright: 2005-6 by the EasyBMP Project       *
*                                                *
* description: Actual source file                *
*                                                *
*************************************************/

#include "bitmap.h"
#include <iostream>
#include <fstream>
/* These functions are defined in EasyBMP.h */
#include <cdtypes.h>
#include <cdlog.h>

using namespace std;

int IntPow( int base, int exponent ) {
    int output = 1;
    for(int i=0 ; i < exponent ; i++ )
        output *= base;
    return output;
}

inline double Square( double number ) {
    return number*number;
}

inline int IntSquare( int number ) {
    return number*number;
}

inline bool IsBigEndian() {
    short word = 0x0001;
    return (*(char *)& word) != 0x01;
}

inline WORD FlipWORD( WORD in ) {
    return ( (in >> 8) | (in << 8) );
}

inline DWORD FlipDWORD(DWORD in ) {
    return ( ((in&0xFF000000)>>24) | ((in&0x000000FF)<<24) |
             ((in&0x00FF0000)>>8 ) | ((in&0x0000FF00)<<8 )   );
}

class BMFH {
  public:
    WORD  bfType;
    DWORD bfSize;
    WORD  bfReserved1;
    WORD  bfReserved2;
    DWORD bfOffBits;

    BMFH() {
        bfType = 19778;
        bfReserved1 = 0;
        bfReserved2 = 0;
    };
    void display( void ) {
        cout << "bfType: " << (int) bfType << endl
             << "bfSize: " << (int) bfSize << endl
             << "bfReserved1: " << (int) bfReserved1 << endl
             << "bfReserved2: " << (int) bfReserved2 << endl
             << "bfOffBits: " << (int) bfOffBits << endl << endl;
    }
    void SwitchEndianess( void ) {
        bfType = FlipWORD( bfType );
        bfSize = FlipDWORD( bfSize );
        bfReserved1 = FlipWORD( bfReserved1 );
        bfReserved2 = FlipWORD( bfReserved2 );
        bfOffBits = FlipDWORD( bfOffBits );
    };
};

class BMIH {
  public:
    DWORD biSize;
    DWORD biWidth;
    DWORD biHeight;
    WORD  biPlanes;
    WORD  biBitCount;
    DWORD biCompression;
    DWORD biSizeImage;
    DWORD biXPelsPerMeter;
    DWORD biYPelsPerMeter;
    DWORD biClrUsed;
    DWORD biClrImportant;

    BMIH() {
        biPlanes = 1;
        biCompression = 0;
        biXPelsPerMeter = DefaultXPelsPerMeter;
        biYPelsPerMeter = DefaultYPelsPerMeter;
        biClrUsed = 0;
        biClrImportant = 0;
    }
    void display( void ) {
        cout << "biSize: " << (int) biSize << endl
             << "biWidth: " << (int) biWidth << endl
             << "biHeight: " << (int) biHeight << endl
             << "biPlanes: " << (int) biPlanes << endl
             << "biBitCount: " << (int) biBitCount << endl
             << "biCompression: " << (int) biCompression << endl
             << "biSizeImage: " << (int) biSizeImage << endl
             << "biXPelsPerMeter: " << (int) biXPelsPerMeter << endl
             << "biYPelsPerMeter: " << (int) biYPelsPerMeter << endl
             << "biClrUsed: " << (int) biClrUsed << endl
             << "biClrImportant: " << (int) biClrImportant << endl << endl;
    }
    void SwitchEndianess( void ) {
        biSize = FlipDWORD( biSize );
        biWidth = FlipDWORD( biWidth );
        biHeight = FlipDWORD( biHeight );
        biPlanes = FlipWORD( biPlanes );
        biBitCount = FlipWORD( biBitCount );
        biCompression = FlipDWORD( biCompression );
        biSizeImage = FlipDWORD( biSizeImage );
        biXPelsPerMeter = FlipDWORD( biXPelsPerMeter );
        biYPelsPerMeter = FlipDWORD( biYPelsPerMeter );
        biClrUsed = FlipDWORD( biClrUsed );
        biClrImportant = FlipDWORD( biClrImportant );
    }
};

/* These functions are defined in EasyBMP_BMP.h */

UINT Bitmap::GetPixel( int i, int j ) const {
    bool Warn = false;
    if( i >= Width ) {
        i = Width-1;
        Warn = true;
    }
    if( i < 0 ) {
        i = 0;
        Warn = true;
    }
    if( j >= Height ) {
        j = Height-1;
        Warn = true;
    }
    if( j < 0 ) {
        j = 0;
        Warn = true;
    }
    LOGW("EasyBMP Warning: Attempted to access non-existent pixel"
               "Truncating request to fit in the range [0,%d]x[0,%d]",Width-1,Height-1);
    return Pixels[i*Width+j];
}

bool Bitmap::SetPixel( int i, int j,UINT NewPixel ) {
    Pixels[i*Width+j]= NewPixel;
    return true;
}

void Bitmap::CopyPixels(void*buf,int stride) {
    UINT*ps=Pixels;
    BYTE*pd=(BYTE*)buf;
    for(int y=0; y<Height; y++) {
        memcpy(pd,((BYTE*)ps),Width*4);
        ps+=Width;
        pd+=stride;
    }
}

bool Bitmap::SetColor( int ColorNumber , RGBAColor NewColor ) {
    if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 ) {
        LOGW("EasyBMP Warning: Attempted to change color table for a BMP object"
                   "that lacks a color table. Ignoring request.");
        return false;
    }
    if( !Colors ) {
        LOGW("EasyBMP Warning: Attempted to set a color, but the color table"
                   "  is not defined. Ignoring request.");
        return false;
    }
    if( ColorNumber >= TellNumberOfColors() ) {
        LOGW("EasyBMP Warning: Requested color number %d is outside the allowed range"
                   " [0,%d]. Ignoring request to set this color." ,ColorNumber, TellNumberOfColors()-1);
        return false;
    }
    Colors[ColorNumber] = NewColor;
    return true;
}

// RGBAColor BMP::GetColor( int ColorNumber ) const
RGBAColor Bitmap::GetColor( int ColorNumber ) {
    RGBAColor Output;
    Output.Red   = 255;
    Output.Green = 255;
    Output.Blue  = 255;
    Output.Alpha = 0;

    if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 ) {
        LOGW("EasyBMP Warning: Attempted to access color table for a BMP object"
                   " that lacks a color table. Ignoring request.");
        return Output;
    }
    if( !Colors ) {
        LOGW("EasyBMP Warning: Requested a color, but the color table"
                   " is not defined. Ignoring request.");
        return Output;
    }
    if( ColorNumber >= TellNumberOfColors() ) {
        LOGW("EasyBMP Warning: Requested color number %d is outside the allowed range[0,%d]"
                   "Ignoring request to get this color.",ColorNumber,TellNumberOfColors()-1);
        return Output;
    }
    return Colors[ColorNumber];
}

UINT Bitmap::RGBA2Pixel(int coloridx) {
    RGBAColor*c=Colors+coloridx;
    return (c->Alpha<<24)|(c->Red<<16)|(c->Green<<8)|c->Blue;
}

Bitmap::Bitmap() {
    Width = 1;
    Height = 1;
    BitDepth = 24;
    Pixels = new UINT[1];
    Colors = NULL;

    XPelsPerMeter = 0;
    YPelsPerMeter = 0;

    MetaData1 = NULL;
    SizeOfMetaData1 = 0;
    MetaData2 = NULL;
    SizeOfMetaData2 = 0;
}

// BMP::BMP( const BMP& Input )
Bitmap::Bitmap( Bitmap& Input ) {
// first, make the image empty.

    Width = 1;
    Height = 1;
    BitDepth = 24;
    Pixels = new UINT[Width];
    Colors = NULL;
    XPelsPerMeter = 0;
    YPelsPerMeter = 0;

    MetaData1 = NULL;
    SizeOfMetaData1 = 0;
    MetaData2 = NULL;
    SizeOfMetaData2 = 0;

// now, set the correct bit depth

    SetBitDepth( Input.TellBitDepth() );

// set the correct pixel size

    SetSize( Input.TellWidth() , Input.TellHeight() );

// set the DPI information from Input

    SetDPI( Input.TellHorizontalDPI() , Input.TellVerticalDPI() );

// if there is a color table, get all the colors

    if( BitDepth == 1 || BitDepth == 4 ||
            BitDepth == 8 ) {
        for( int k=0 ; k < TellNumberOfColors() ; k++ ) {
            SetColor( k, Input.GetColor( k ));
        }
    }

// get all the pixels
    UINT*pd=Pixels;
    UINT*ps=Input.Pixels;
    for( int j=0; j < Height ; j++ ) {
        memcpy(pd,ps,Input.Width*4);
        pd+=Width;
        ps+=Input.Width;
    }
}

Bitmap::~Bitmap() {
    int i;
    if( Colors ) {
        delete [] Colors;
    }

    if( MetaData1 ) {
        delete [] MetaData1;
    }
    if( MetaData2 ) {
        delete [] MetaData2;
    }
}

// int BMP::TellBitDepth( void ) const
int Bitmap::TellBitDepth( void ) {
    return BitDepth;
}

// int BMP::TellHeight( void ) const
int Bitmap::TellHeight( void ) {
    return Height;
}

// int BMP::TellWidth( void ) const
int Bitmap::TellWidth( void ) {
    return Width;
}

// int BMP::TellNumberOfColors( void ) const
int Bitmap::TellNumberOfColors( void ) {
    int output = IntPow( 2, BitDepth );
    if( BitDepth == 32 ) {
        output = IntPow( 2, 24 );
    }
    return output;
}

bool Bitmap::SetBitDepth( int NewDepth ) {
    using namespace std;
    if( NewDepth != 1 && NewDepth != 4 &&
            NewDepth != 8 && NewDepth != 16 &&
            NewDepth != 24 && NewDepth != 32 ) {
        LOGW("EasyBMP Warning: User attempted to set unsupported bit depth %d Bit depth remains unchanged at %d"
                   ,NewDepth,BitDepth);
        return false;
    }

    BitDepth = NewDepth;
    if( Colors ) {
        delete [] Colors;
    }
    int NumberOfColors = IntPow( 2, BitDepth );
    if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 ) {
        Colors = new RGBAColor [NumberOfColors];
    } else {
        Colors = NULL;
    }
    if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 ) {
        CreateStandardColorTable();
    }
    return true;
}

bool Bitmap::SetSize(int NewWidth , int NewHeight ) {
    using namespace std;
    if( NewWidth <= 0 || NewHeight <= 0 ) {
        LOGW("EasyBMP Warning: User attempted to set a non-positive width or height."
                   " Size remains unchanged at %dx%d",Width,Height);
        return false;
    }

    int i,j;

    Width = NewWidth;
    Height = NewHeight;
    image=ImageSurface::create(Surface::Format::RGB24,Width,Height);
    Pixels=(UINT*)image->get_data();
    memset(Pixels,0xFF,Width*Height*4);
    return true;
}

Bitmap::operator RefPtr<ImageSurface>(){
    return image;
}

bool Bitmap::WriteToStream(std::ostream&os) {
    using namespace std;
    if(!os.good()) {
        LOGI("EasyBMP Error: Cannot open file stream for output.");
        return false;
    }

// some preliminaries

    double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
    double dBytesPerRow = dBytesPerPixel * (Width+0.0);
    dBytesPerRow = ceil(dBytesPerRow);

    int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) )% 4;
    if( BytePaddingPerRow == 4 ) {
        BytePaddingPerRow = 0;
    }

    double dActualBytesPerRow = dBytesPerRow + BytePaddingPerRow;

    double dTotalPixelBytes = Height * dActualBytesPerRow;

    double dPaletteSize = 0;
    if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 ) {
        dPaletteSize = IntPow(2,BitDepth)*4.0;
    }

// leave some room for 16-bit masks
    if( BitDepth == 16 ) {
        dPaletteSize = 3*4;
    }

    double dTotalFileSize = 14 + 40 + dPaletteSize + dTotalPixelBytes;

// write the file header

    BMFH bmfh;
    bmfh.bfSize = (DWORD) dTotalFileSize;
    bmfh.bfReserved1 = 0;
    bmfh.bfReserved2 = 0;
    bmfh.bfOffBits = (DWORD) (14+40+dPaletteSize);

    if( IsBigEndian() ) {
        bmfh.SwitchEndianess();
    }

    os.write( (char*) &(bmfh.bfType) , sizeof(WORD) );
    os.write( (char*) &(bmfh.bfSize) , sizeof(DWORD) );
    os.write( (char*) &(bmfh.bfReserved1),sizeof(WORD));
    os.write( (char*) &(bmfh.bfReserved2),sizeof(WORD));
    os.write( (char*) &(bmfh.bfOffBits) , sizeof(DWORD));

// write the info header

    BMIH bmih;
    bmih.biSize = 40;
    bmih.biWidth = Width;
    bmih.biHeight = Height;
    bmih.biPlanes = 1;
    bmih.biBitCount = BitDepth;
    bmih.biCompression = 0;
    bmih.biSizeImage = (DWORD) dTotalPixelBytes;
    if( XPelsPerMeter ) {
        bmih.biXPelsPerMeter = XPelsPerMeter;
    } else {
        bmih.biXPelsPerMeter = DefaultXPelsPerMeter;
    }
    if( YPelsPerMeter ) {
        bmih.biYPelsPerMeter = YPelsPerMeter;
    } else {
        bmih.biYPelsPerMeter = DefaultYPelsPerMeter;
    }

    bmih.biClrUsed = 0;
    bmih.biClrImportant = 0;

// indicates that we'll be using bit fields for 16-bit files
    if( BitDepth == 16 ) {
        bmih.biCompression = 3;
    }

    if( IsBigEndian() ) {
        bmih.SwitchEndianess();
    }

    os.write( (char*) &(bmih.biSize) , sizeof(DWORD) );
    os.write( (char*) &(bmih.biWidth) , sizeof(DWORD) );
    os.write( (char*) &(bmih.biHeight) , sizeof(DWORD));
    os.write( (char*) &(bmih.biPlanes) , sizeof(WORD));
    os.write( (char*) &(bmih.biBitCount) , sizeof(WORD));
    os.write( (char*) &(bmih.biCompression) , sizeof(DWORD));
    os.write( (char*) &(bmih.biSizeImage) , sizeof(DWORD));
    os.write( (char*) &(bmih.biXPelsPerMeter) , sizeof(DWORD));
    os.write( (char*) &(bmih.biYPelsPerMeter) , sizeof(DWORD));
    os.write( (char*) &(bmih.biClrUsed) , sizeof(DWORD));
    os.write( (char*) &(bmih.biClrImportant) , sizeof(DWORD));

// write the palette
    if( BitDepth == 1 || BitDepth == 4 || BitDepth == 8 ) {
        int NumberOfColors = IntPow(2,BitDepth);

        // if there is no palette, create one
        if( !Colors ) {
            if( !Colors ) {
                Colors = new RGBAColor [NumberOfColors];
            }
            CreateStandardColorTable();
        }

        int n;
        for( n=0 ; n < NumberOfColors ; n++ ) {
            os.write( (char*) &(Colors[n]) , 4 );
        }
    }

// write the pixels
    int i,j;
    if( BitDepth != 16 ) {
        BYTE* Buffer;
        int BufferSize = (int) ( (Width*BitDepth)/8.0 );
        while( 8*BufferSize < Width*BitDepth ) {
            BufferSize++;
        }
        while( BufferSize % 4 ) {
            BufferSize++;
        }

        Buffer = new BYTE [BufferSize];
        for( j=0 ; j < BufferSize; j++ ) {
            Buffer[j] = 0;
        }

        j=Height-1;

        while( j > -1 ) {
            bool Success = false;
            if( BitDepth == 32 ) {
                Success = Write32bitRow( Buffer, BufferSize,j);
            }
            if( BitDepth == 24 ) {
                Success = Write24bitRow( Buffer, BufferSize,j);
            }
            if( BitDepth == 8  ) {
                Success = Write8bitRow( Buffer, BufferSize, j);
            }
            if( BitDepth == 4  ) {
                Success = Write4bitRow( Buffer, BufferSize, j);
            }
            if( BitDepth == 1  ) {
                Success = Write1bitRow( Buffer, BufferSize, j);
            }
            if( Success ) {
                os.write( (char*) Buffer,BufferSize);
            }
            if( !Success ) {
                LOGI("EasyBMP Error: Could not write proper amount of data.");
                j = -1;
            }
            j--;
        }

        delete [] Buffer;
    }

    if( BitDepth == 16 ) {
        // write the bit masks

        WORD BlueMask = 31;    // bits 12-16
        WORD GreenMask = 2016; // bits 6-11
        WORD RedMask = 63488;  // bits 1-5
        WORD ZeroWORD;

        if( IsBigEndian() ) {
            RedMask = FlipWORD( RedMask );
        }
        os.write( (char*) &RedMask , 2);
        os.write( (char*) &ZeroWORD ,2);

        if( IsBigEndian() ) {
            GreenMask = FlipWORD( GreenMask );
        }
        os.write( (char*) &GreenMask ,2);
        os.write( (char*) &ZeroWORD , 2);

        if( IsBigEndian() ) {
            BlueMask = FlipWORD( BlueMask );
        }
        os.write( (char*) &BlueMask , 2);
        os.write( (char*) &ZeroWORD , 2);

        int DataBytes = Width*2;
        int PaddingBytes = ( 4 - DataBytes % 4 ) % 4;

        // write the actual pixels

        for( j=Height-1 ; j >= 0 ; j-- ) {
            // write all row pixel data
            i=0;
            int WriteNumber = 0;
            while( WriteNumber < DataBytes ) {
                WORD TempWORD;
                BYTE *p=((BYTE*)Pixels)+(j*Width+i*4);
                WORD RedWORD = (WORD) (p[0]>>3);
                WORD GreenWORD = (WORD) (p[1]>>2);
                WORD BlueWORD = (WORD) (p[2]>>3);

                TempWORD = (RedWORD<<11) + (GreenWORD<<5) + BlueWORD;
                if( IsBigEndian() ) {
                    TempWORD = FlipWORD( TempWORD );
                }

                os.write( (char*) &TempWORD,2);
                WriteNumber += 2;
                i++;
            }
            // write any necessary row padding
            WriteNumber = 0;
            while( WriteNumber < PaddingBytes ) {
                BYTE TempBYTE;
                os.write( (char*) &TempBYTE ,1);
                WriteNumber++;
            }
        }

    }
    return true;
}

bool Bitmap::ReadFromStream(std::istream&is) {
    using namespace std;
    if( !is.good() ) {
        LOGW("EasyBMP Error: Cannot open stream for input.");
        SetBitDepth(1);
        SetSize(1,1);
        return false;
    }

// read the file header

    BMFH bmfh;
    bool NotCorrupted = true;

    NotCorrupted &= SafeRead( (char*) &(bmfh.bfType) , sizeof(WORD), 1, is);

    bool IsBitmap = false;

    if( IsBigEndian() && bmfh.bfType == 0x424d ) {
        IsBitmap = true;
    }
    if( !IsBigEndian() && bmfh.bfType ==0x4d42 ) {
        IsBitmap = true;
    }

    if( !IsBitmap ) {
        LOGW("EasyBMP Error:stream is not a Windows BMP file!");
        return false;
    }

    NotCorrupted &= SafeRead( (char*) &(bmfh.bfSize) , sizeof(DWORD) , 1, is);
    NotCorrupted &= SafeRead( (char*) &(bmfh.bfReserved1) , sizeof(WORD) , 1, is);
    NotCorrupted &= SafeRead( (char*) &(bmfh.bfReserved2) , sizeof(WORD) , 1, is);
    NotCorrupted &= SafeRead( (char*) &(bmfh.bfOffBits) , sizeof(DWORD) , 1 , is);

    if( IsBigEndian() ) {
        bmfh.SwitchEndianess();
    }

// read the info header

    BMIH bmih;

    NotCorrupted &= SafeRead( (char*) &(bmih.biSize) , sizeof(DWORD) , 1 , is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biWidth) , sizeof(DWORD) , 1 ,is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biHeight) , sizeof(DWORD) , 1 ,is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biPlanes) , sizeof(WORD) , 1, is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biBitCount) , sizeof(WORD) , 1, is);

    NotCorrupted &= SafeRead( (char*) &(bmih.biCompression),sizeof(DWORD) ,1,is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biSizeImage) , sizeof(DWORD) ,1,is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biXPelsPerMeter) , sizeof(DWORD),1,is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biYPelsPerMeter) , sizeof(DWORD),1,is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biClrUsed) , sizeof(DWORD) , 1 , is);
    NotCorrupted &= SafeRead( (char*) &(bmih.biClrImportant) , sizeof(DWORD) ,1,is);

    if( IsBigEndian() ) {
        bmih.SwitchEndianess();
    }

// a safety catch: if any of the header information didn't read properly, abort
// future idea: check to see if at least most is self-consistent

    if( !NotCorrupted ) {
        LOGW("EasyBMP Error: is obviously corrupted.");
        SetSize(1,1);
        SetBitDepth(1);
        return false;
    }

    XPelsPerMeter = bmih.biXPelsPerMeter;
    YPelsPerMeter = bmih.biYPelsPerMeter;

// if bmih.biCompression 1 or 2, then the file is RLE compressed

    if( bmih.biCompression == 1 || bmih.biCompression == 2 ) {
        LOGW("EasyBMP Error: stream is (RLE) compressed."
                   " EasyBMP does not support compression.");
        SetSize(1,1);
        SetBitDepth(1);
        return false;
    }

// if bmih.biCompression > 3, then something strange is going on
// it's probably an OS2 bitmap file.

    if( bmih.biCompression > 3 ) {
        LOGW("EasyBMP Error: stream is in an unsupported format. (bmih.biCompression = %d)"
                   " The file is probably an old OS2 bitmap or corrupted.",bmih.biCompression);
        SetSize(1,1);
        SetBitDepth(1);
        return false;
    }

    if( bmih.biCompression == 3 && bmih.biBitCount != 16 ) {
        LOGW("EasyBMP Error: stream uses bit fields and is not a 16-bit file. This is not supported.");
        SetSize(1,1);
        SetBitDepth(1);
        return false;
    }

// set the bit depth

    int TempBitDepth = (int) bmih.biBitCount;
    if(    TempBitDepth != 1  && TempBitDepth != 4
            && TempBitDepth != 8  && TempBitDepth != 16
            && TempBitDepth != 24 && TempBitDepth != 32 ) {
        LOGW("EasyBMP Error: unrecognized bit depth.");
        SetSize(1,1);
        SetBitDepth(1);
        return false;
    }
    SetBitDepth( (int) bmih.biBitCount );

// set the size

    if( (int) bmih.biWidth <= 0 || (int) bmih.biHeight <= 0 ) {
        LOGW("EasyBMP Error: non-positive width or height.");
        SetSize(1,1);
        SetBitDepth(1);
        return false;
    }
    SetSize( (int) bmih.biWidth , (int) bmih.biHeight );

// some preliminaries

    double dBytesPerPixel = ( (double) BitDepth ) / 8.0;
    double dBytesPerRow = dBytesPerPixel * (Width+0.0);
    dBytesPerRow = ceil(dBytesPerRow);

    int BytePaddingPerRow = 4 - ( (int) (dBytesPerRow) )% 4;
    if( BytePaddingPerRow == 4 ) {
        BytePaddingPerRow = 0;
    }

// if < 16 bits, read the palette

    if( BitDepth < 16 ) {
        // determine the number of colors specified in the
        // color table

        int NumberOfColorsToRead = ((int) bmfh.bfOffBits - 54 )/4;
        if( NumberOfColorsToRead > IntPow(2,BitDepth) ) {
            NumberOfColorsToRead = IntPow(2,BitDepth);
        }

        LOGW_IF(NumberOfColorsToRead < TellNumberOfColors(),
                      "EasyBMP Warning:  underspecified color table. The table will be padded with extra"
                      " white (255,255,255,0) entries.");

        int n;
        for( n=0; n < NumberOfColorsToRead ; n++ ) {
            SafeRead( (char*) &(Colors[n]) , 4 , 1 ,is);
        }
        for( n=NumberOfColorsToRead ; n < TellNumberOfColors() ; n++ ) {
            RGBAColor WHITE;
            WHITE.Red = 255;
            WHITE.Green = 255;
            WHITE.Blue = 255;
            WHITE.Alpha = 255;
            SetColor( n , WHITE );
        }


    }

// skip blank data if bfOffBits so indicates

    int BytesToSkip = bmfh.bfOffBits - 54;;
    if( BitDepth < 16 ) {
        BytesToSkip -= 4*IntPow(2,BitDepth);
    }
    if( BitDepth == 16 && bmih.biCompression == 3 ) {
        BytesToSkip -= 3*4;
    }
    if( BytesToSkip < 0 ) {
        BytesToSkip = 0;
    }
    if( BytesToSkip > 0 && BitDepth != 16 ) {
        LOGW("EasyBMP Warning: Extra meta data detected  Data will be skipped.");
        BYTE* TempSkipBYTE;
        TempSkipBYTE = new BYTE [BytesToSkip];
        SafeRead( (char*) TempSkipBYTE , BytesToSkip , 1 ,is);
        delete [] TempSkipBYTE;
    }

// This code reads 1, 4, 8, 24, and 32-bpp files
// with a more-efficient buffered technique.

    int i,j;
    if( BitDepth != 16 ) {
        int BufferSize = (int) ( (Width*BitDepth) / 8.0 );
        while( 8*BufferSize < Width*BitDepth ) {
            BufferSize++;
        }
        BufferSize=(BufferSize+3)&(~3);
        BYTE* Buffer;
        Buffer = new BYTE [BufferSize];
        j= Height-1;
        while( j > -1 ) {
            int BytesRead=is.read( (char*) Buffer,BufferSize).gcount();
            if( BytesRead < BufferSize ) {
                j = -1;
                LOGW("EasyBMP Error: Could not read proper amount of data.");
            } else {
                bool Success = false;
                if( BitDepth == 1  ) {
                    Success = Read1bitRow(  Buffer, BufferSize, j );
                }
                if( BitDepth == 4  ) {
                    Success = Read4bitRow(  Buffer, BufferSize, j );
                }
                if( BitDepth == 8  ) {
                    Success = Read8bitRow(  Buffer, BufferSize, j );
                }
                if( BitDepth == 24 ) {
                    Success = Read24bitRow( Buffer, BufferSize, j );
                }
                if( BitDepth == 32 ) {
                    Success = Read32bitRow( Buffer, BufferSize, j );
                }
                if( !Success ) {
                    LOGW("EasyBMP Error: Could not read enough pixel data!");
                    j = -1;
                }
            }
            j--;
        }
        delete [] Buffer;
    }

    if( BitDepth == 16 ) {
        int DataBytes = Width*2;
        int PaddingBytes = ( 4 - DataBytes % 4 ) % 4;

        // set the default mask

        WORD BlueMask = 31; // bits 12-16
        WORD GreenMask = 992; // bits 7-11
        WORD RedMask = 31744; // bits 2-6

        // read the bit fields, if necessary, to
        // override the default 5-5-5 mask

        if( bmih.biCompression != 0 ) {
            // read the three bit masks

            WORD TempMaskWORD;
            WORD ZeroWORD;

            SafeRead( (char*) &RedMask , 2 , 1 ,is);
            if( IsBigEndian() ) {
                RedMask = FlipWORD(RedMask);
            }
            SafeRead( (char*) &TempMaskWORD ,2,1,is);

            SafeRead( (char*) &GreenMask , 2 ,1 ,is);
            if( IsBigEndian() ) {
                GreenMask = FlipWORD(GreenMask);
            }
            SafeRead( (char*) &TempMaskWORD , 2, 1,is);

            SafeRead( (char*) &BlueMask , 2 , 1 ,is);
            if( IsBigEndian() ) {
                BlueMask = FlipWORD(BlueMask);
            }
            SafeRead( (char*) &TempMaskWORD , 2, 1,is);
        }

        // read and skip any meta data

        if( BytesToSkip > 0 ) {
            LOGW("EasyBMP Warning: Extra meta data detected in file Data will be skipped.");
            BYTE* TempSkipBYTE;
            TempSkipBYTE = new BYTE [BytesToSkip];
            SafeRead( (char*) TempSkipBYTE , BytesToSkip , 1 ,is);
            delete [] TempSkipBYTE;
        }

        // determine the red, green and blue shifts

        int GreenShift = 0;
        WORD TempShiftWORD = GreenMask;
        while( TempShiftWORD > 31 ) {
            TempShiftWORD = TempShiftWORD>>1;
            GreenShift++;
        }
        int BlueShift = 0;
        TempShiftWORD = BlueMask;
        while( TempShiftWORD > 31 ) {
            TempShiftWORD = TempShiftWORD>>1;
            BlueShift++;
        }
        int RedShift = 0;
        TempShiftWORD = RedMask;
        while( TempShiftWORD > 31 ) {
            TempShiftWORD = TempShiftWORD>>1;
            RedShift++;
        }

        // read the actual pixels

        for( j=Height-1 ; j >= 0 ; j-- ) {
            i=0;
            BYTE*pp=(BYTE*)(Pixels+j*Width);
            int ReadNumber = 0;
            while( ReadNumber < DataBytes ) {
                WORD TempWORD;
                SafeRead( (char*) &TempWORD , 2 , 1 ,is);
                if( IsBigEndian() ) {
                    TempWORD = FlipWORD(TempWORD);
                }
                ReadNumber += 2;

                WORD Red = RedMask & TempWORD;
                WORD Green = GreenMask & TempWORD;
                WORD Blue = BlueMask & TempWORD;

                BYTE BlueBYTE = (BYTE) 8*(Blue>>BlueShift);
                BYTE GreenBYTE = (BYTE) 8*(Green>>GreenShift);
                BYTE RedBYTE = (BYTE) 8*(Red>>RedShift);

                *pp++= RedBYTE;
                *pp++= GreenBYTE;
                *pp++= BlueBYTE;
                *pp++=0xFF;//ALPHA;
                i++;
            }
            ReadNumber = 0;
            while( ReadNumber < PaddingBytes ) {
                BYTE TempBYTE;
                SafeRead( (char*) &TempBYTE , 1, 1,is);
                ReadNumber++;
            }
        }

    }
    return true;
}

bool Bitmap::CreateStandardColorTable( void ) {
    using namespace std;
    if( BitDepth != 1 && BitDepth != 4 && BitDepth != 8 ) {
        LOGW("EasyBMP Warning: Attempted to create color table at a bit"
                   " depth that does not require a color table.Ignoring request.");
        return false;
    }

    if( BitDepth == 1 ) {
        int i;
        for( i=0 ; i < 2 ; i++ ) {
            Colors[i].Red = i*255;
            Colors[i].Green = i*255;
            Colors[i].Blue = i*255;
            Colors[i].Alpha = 0;
        }
        return true;
    }

    if( BitDepth == 4 ) {
        int i = 0;
        int j,k,ell;

        // simplify the code for the first 8 colors
        for( ell=0 ; ell < 2 ; ell++ ) {
            for( k=0 ; k < 2 ; k++ ) {
                for( j=0 ; j < 2 ; j++ ) {
                    Colors[i].Red = j*128;
                    Colors[i].Green = k*128;
                    Colors[i].Blue = ell*128;
                    i++;
                }
            }
        }

        // simplify the code for the last 8 colors
        for( ell=0 ; ell < 2 ; ell++ ) {
            for( k=0 ; k < 2 ; k++ ) {
                for( j=0 ; j < 2 ; j++ ) {
                    Colors[i].Red = j*255;
                    Colors[i].Green = k*255;
                    Colors[i].Blue = ell*255;
                    i++;
                }
            }
        }

        // overwrite the duplicate color
        i=8;
        Colors[i].Red = 192;
        Colors[i].Green = 192;
        Colors[i].Blue = 192;

        for( i=0 ; i < 16 ; i++ ) {
            Colors[i].Alpha = 0;
        }
        return true;
    }

    if( BitDepth == 8 ) {
        int i=0;
        int j,k,ell;

        // do an easy loop, which works for all but colors
        // 0 to 9 and 246 to 255
        for( ell=0 ; ell < 4 ; ell++ ) {
            for( k=0 ; k < 8 ; k++ ) {
                for( j=0; j < 8 ; j++ ) {
                    Colors[i].Red = j*32;
                    Colors[i].Green = k*32;
                    Colors[i].Blue = ell*64;
                    Colors[i].Alpha = 0;
                    i++;
                }
            }
        }

        // now redo the first 8 colors
        i=0;
        for( ell=0 ; ell < 2 ; ell++ ) {
            for( k=0 ; k < 2 ; k++ ) {
                for( j=0; j < 2 ; j++ ) {
                    Colors[i].Red = j*128;
                    Colors[i].Green = k*128;
                    Colors[i].Blue = ell*128;
                    i++;
                }
            }
        }

        // overwrite colors 7, 8, 9
        i=7;
        Colors[i].Red = 192;
        Colors[i].Green = 192;
        Colors[i].Blue = 192;
        i++; // 8
        Colors[i].Red = 192;
        Colors[i].Green = 220;
        Colors[i].Blue = 192;
        i++; // 9
        Colors[i].Red = 166;
        Colors[i].Green = 202;
        Colors[i].Blue = 240;

        // overwrite colors 246 to 255
        i=246;
        Colors[i].Red = 255;
        Colors[i].Green = 251;
        Colors[i].Blue = 240;
        i++; // 247
        Colors[i].Red = 160;
        Colors[i].Green = 160;
        Colors[i].Blue = 164;
        i++; // 248
        Colors[i].Red = 128;
        Colors[i].Green = 128;
        Colors[i].Blue = 128;
        i++; // 249
        Colors[i].Red = 255;
        Colors[i].Green = 0;
        Colors[i].Blue = 0;
        i++; // 250
        Colors[i].Red = 0;
        Colors[i].Green = 255;
        Colors[i].Blue = 0;
        i++; // 251
        Colors[i].Red = 255;
        Colors[i].Green = 255;
        Colors[i].Blue = 0;
        i++; // 252
        Colors[i].Red = 0;
        Colors[i].Green = 0;
        Colors[i].Blue = 255;
        i++; // 253
        Colors[i].Red = 255;
        Colors[i].Green = 0;
        Colors[i].Blue = 255;
        i++; // 254
        Colors[i].Red = 0;
        Colors[i].Green = 255;
        Colors[i].Blue = 255;
        i++; // 255
        Colors[i].Red = 255;
        Colors[i].Green = 255;
        Colors[i].Blue = 255;

        return true;
    }
    return true;
}

bool Bitmap::SafeRead( char* buffer, int size, int number,std::istream&is) {
    if(!is.good()) {
        return false;
    }
    int ItemsRead=is.read(buffer , size*number).gcount();
    return ItemsRead==number*size;
}

void Bitmap::SetDPI( int HorizontalDPI, int VerticalDPI ) {
    XPelsPerMeter = (int) ( HorizontalDPI * 39.37007874015748 );
    YPelsPerMeter = (int) (   VerticalDPI * 39.37007874015748 );
}

// int BMP::TellVerticalDPI( void ) const
int Bitmap::TellVerticalDPI( void ) {
    if( !YPelsPerMeter ) {
        YPelsPerMeter = DefaultYPelsPerMeter;
    }
    return (int) ( YPelsPerMeter / (double) 39.37007874015748 );
}

// int BMP::TellHorizontalDPI( void ) const
int Bitmap::TellHorizontalDPI( void ) {
    if( !XPelsPerMeter ) {
        XPelsPerMeter = DefaultXPelsPerMeter;
    }
    return (int) ( XPelsPerMeter / (double) 39.37007874015748 );
}

bool Bitmap::Read32bitRow( BYTE* Buffer, int BufferSize, int Row ) {
    int i;
    if( Width*4 > BufferSize ) {
        return false;
    }
    memcpy( (char*) &(Pixels[Row*Width]), (char*) Buffer+4*i, 4 );
    return true;
}

bool Bitmap::Read24bitRow( BYTE* Buffer, int BufferSize, int Row ) {
    BYTE*pp=(BYTE*)(Pixels+Row*Width);
    for(int i=0 ; i < Width ; i++,pp+=4,Buffer+=3 ) {
        pp[3]=0xFF;
        memcpy(pp, Buffer, 3 );
    }
    return true;
}

bool Bitmap::Read8bitRow(  BYTE* Buffer, int BufferSize, int Row ) {
    UINT*pp=Pixels+Row*Width;
    for(int i=0 ; i < Width ; i++ ) {
        int Index = Buffer[i];
        pp[i]=RGBA2Pixel(Index);
    }
    return true;
}

bool Bitmap::Read4bitRow(  BYTE* Buffer, int BufferSize, int Row ) {
    int Shifts[2] = {4  ,0 };
    int Masks[2]  = {240,15};

    int i=0;
    int j;
    int k=0;
    if( Width > 2*BufferSize ) {
        return false;
    }
    while( i < Width ) {
        j=0;
        UINT*pp=Pixels+Row*Width;
        while( j < 2 && i < Width ) {
            int Index = (int) ( (Buffer[k]&Masks[j]) >> Shifts[j]);
            pp[i++]=RGBA2Pixel(Index);
            j++;
        }
        k++;
    }
    return true;
}
bool Bitmap::Read1bitRow(  BYTE* Buffer, int BufferSize, int Row ) {
    int Shifts[8] = {7  ,6 ,5 ,4 ,3,2,1,0};
    int Masks[8]  = {128,64,32,16,8,4,2,1};

    int i=0;
    int j;
    int k=0;

    if( Width > 8*BufferSize ) {
        return false;
    }
    while( i < Width ) {
        j=0;
        UINT*pp=Pixels+Row*Width;
        while( j < 8 && i < Width ) {
            int Index = (int) ( (Buffer[k]&Masks[j]) >> Shifts[j]);
            pp[i++]= RGBA2Pixel(Index);
            j++;
        }
        k++;
    }
    return true;
}

bool Bitmap::Write32bitRow( BYTE* Buffer, int BufferSize, int Row ) {
    int i;
    if( Width*4 > BufferSize ) {
        return false;
    }
    UINT*pp=Pixels+Row*Width;
    for( i=0 ; i < Width ; i++,Buffer+=4,pp++ ) {
        memcpy( (char*)Buffer,(char*)pp, 4 );
    }
    return true;
}

bool Bitmap::Write24bitRow( BYTE* Buffer, int BufferSize, int Row ) {
    int i;
    if( Width*3 > BufferSize ) {
        return false;
    }
    BYTE*pp=(BYTE*)(Pixels+Row*Width);
    for( i=0 ; i < Width ; i++,pp+=4 ) {
        memcpy( (char*) Buffer+3*i, pp+1, 3 );
    }
    return true;
}

bool Bitmap::Write8bitRow(  BYTE* Buffer, int BufferSize, int Row ) {
    int i;
    if( Width > BufferSize ) {
        return false;
    }
    BYTE*pp=(BYTE*)(Pixels+Row*Width);
    for( i=0 ; i < Width ; i++,pp+=4 ) {
        RGBAColor c= {pp[i],pp[i+1],pp[i+2],0};
        Buffer[i] = FindClosestColor(c);
    }
    return true;
}

bool Bitmap::Write4bitRow(  BYTE* Buffer, int BufferSize, int Row ) {
    int PositionWeights[2]  = {16,1};

    int i=0;
    int j;
    int k=0;
    if( Width > 2*BufferSize ) {
        return false;
    }
    while( i < Width ) {
        j=0;
        int Index = 0;
        BYTE*pp=(BYTE*)(Pixels+Row*Width);
        while( j < 2 && i < Width ) {
            RGBAColor c= {pp[i],pp[i+1],pp[i+2],0};
            Index += ( PositionWeights[j]* (int) FindClosestColor(c) );
            i++;
            j++;
        }
        Buffer[k] = (BYTE) Index;
        k++;
    }
    return true;
}

bool Bitmap::Write1bitRow(  BYTE* Buffer, int BufferSize, int Row ) {
    int PositionWeights[8]  = {128,64,32,16,8,4,2,1};

    int i=0;
    int j;
    int k=0;
    if( Width > 8*BufferSize ) {
        return false;
    }
    while( i < Width ) {
        j=0;
        int Index = 0;
        BYTE*pp=(BYTE*)(Pixels+Row*Width);
        while( j < 8 && i < Width ) {
            RGBAColor c= {pp[i],pp[i+1],pp[i+2],0};
            Index += ( PositionWeights[j]* (int) FindClosestColor(c) );
            i++;
            j++;
        }
        Buffer[k] = (BYTE) Index;
        k++;
    }
    return true;
}

BYTE Bitmap::FindClosestColor( RGBAColor& input ) {
    using namespace std;

    int i=0;
    int NumberOfColors = TellNumberOfColors();
    BYTE BestI = 0;
    int BestMatch = 999999;

    while( i < NumberOfColors ) {
        RGBAColor Attempt = GetColor( i );
        int TempMatch = IntSquare( (int) Attempt.Red - (int) input.Red )
                        + IntSquare( (int) Attempt.Green - (int) input.Green )
                        + IntSquare( (int) Attempt.Blue - (int) input.Blue );
        if( TempMatch < BestMatch ) {
            BestI = (BYTE) i;
            BestMatch = TempMatch;
        }
        if( BestMatch < 1 ) {
            i = NumberOfColors;
        }
        i++;
    }
    return BestI;
}



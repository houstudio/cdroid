#ifndef __APNG_CHUNKS_H__
#define __APNG_CHUNKS_H__
#include <iostream>
namespace cdroid{
class Chunk {
public:
    int length;
    int fourcc;
    int crc;
    int offset;
public:
    static constexpr int fourCCToInt(const char* fourCC){
         return (fourCC[0] & 0xff) | ((fourCC[1] & 0xff) << 8)
            | ((fourCC[2] & 0xff) << 16)
            | ((fourCC[3] & 0xff) << 24);
    }
    static int readInt(std::istream&);
    static short readShort(std::istream&);
    void parse(std::istream& reader);
    virtual void innerParse(std::istream& reader);
};

class ACTLChunk :public Chunk {
public:
    static constexpr int ID = fourCCToInt("acTL");
    int num_frames;
    int num_plays;

    void innerParse(std::istream& apngReader)override;
};

class FCTLChunk :public Chunk {
public:
    static constexpr int ID = fourCCToInt("fcTL");
    int sequence_number;
    int width;
    int height;
    int x_offset;
    int y_offset;
    short delay_num;
    short delay_den;
    uint8_t dispose_op;
    uint8_t blend_op;
    static constexpr int APNG_DISPOSE_OP_NON = 0;
    static constexpr int APNG_DISPOSE_OP_BACKGROUND = 1;
    static constexpr int APNG_DISPOSE_OP_PREVIOUS = 2;
    static constexpr int APNG_BLEND_OP_SOURCE = 0;
    static constexpr int APNG_BLEND_OP_OVER = 1;

    void innerParse(std::istream& reader)override;
};

class FDATChunk :public Chunk {
public:
    static constexpr int ID = fourCCToInt("fdAT");
    int sequence_number;
    void innerParse(std::istream& reader)override;
};

class IDATChunk :public Chunk {
public:
    static constexpr int ID = fourCCToInt("IDAT");
};

class IENDChunk :public Chunk {
public:
    static constexpr int ID = fourCCToInt("IEND");
};

class IHDRChunk :public Chunk {
public:
    static constexpr int ID = fourCCToInt("IHDR");
    int width;
    int height;
    uint8_t data[5];

    void innerParse(std::istream& reader)override;
};

}/*endof namespace*/
#endif/*__APNG_CHUNKS_H__*/

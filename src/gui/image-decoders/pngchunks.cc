#include <pngchunks.h>
namespace cdroid{

void Chunk::parse(std::istream& reader){
    //int available = reader.available();
    innerParse(reader);
    /*int offset = available - reader.available();
    if (offset > length) {
        throw new IOException("Out of chunk area");
    } else if (offset < length) {
        reader.skip(length - offset);
    }*/
}

int Chunk::readInt(std::istream&is){
    uint8_t bytes[4];
    is.read((char*)bytes,4);
    return (bytes[0]<<24)|(bytes[1]<<16)|(bytes[2]<<8)|bytes[3];
}

short Chunk::readShort(std::istream&is){
    uint8_t bytes[4];
    is.read((char*)bytes,2);
    return (bytes[1]<<8)|bytes[1];
}

void Chunk::innerParse(std::istream& reader){
}

//////////////////////////////////////////////////////////////////////

void ACTLChunk::innerParse(std::istream& apngReader){
    num_frames = readInt(apngReader);//.readInt();
    num_plays = readInt(apngReader);//.readInt();
}

/////////////////////////////////////////////////////////////////////

void FCTLChunk::innerParse(std::istream& reader){
    sequence_number = readInt(reader);//.readInt();
    width = readInt(reader);//.readInt();
    height = readInt(reader);//.readInt();
    x_offset = readInt(reader);//.readInt();
    y_offset = readInt(reader);//.readInt();
    delay_num = readInt(reader);//.readShort();
    delay_den = readInt(reader);//.readShort();
    reader.read((char*)&dispose_op,1);//dispose_op = reader.peek();
    reader.read((char*)&blend_op,1);// = reader.peek();
}

//////////////////////////////////////////////////////////////////////

void FDATChunk::innerParse(std::istream& reader){
    sequence_number = readInt(reader);//.readInt();
}

//////////////////////////////////////////////////////////////////////

void IHDRChunk::innerParse(std::istream& reader){
    width = readInt(reader);//.readInt();
    height = readInt(reader);//.readInt();
    reader.read((char*)data,sizeof(data));////reader.read(data, 0, data.length);
}

}/*endof namespace*/

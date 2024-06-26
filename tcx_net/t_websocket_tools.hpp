#pragma once

#include <stdint.h>
#include <string>
#include <optional>
#include <limits>
#include "../tcx_standalone/t_blob.hpp"

#define __TCX_WS_GUID "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"

namespace tcx{

struct WSFrameHead125{
    unsigned char FIN : 1;
    unsigned char RSV1 :1;
    unsigned char RSV2: 1;
    unsigned char RSV3: 1;
    unsigned char opcode : 4;
    unsigned char mask = 1;
    unsigned char payload_len = 7;
};

struct WSFrameHead126 : WSFrameHead125{
    unsigned int payload_len_ex;
};

struct WSFrameHead127 : WSFrameHead125{
    unsigned long long payload_len_ex;
};

struct WSFrame{
    bool FIN;
    unsigned char opcode;
    bool mask;
    unsigned long long payload_len;
    unsigned int masking_key;
    Blob payload;
};

namespace{

template<typename T>
void reverse_binary(T & data){
    unsigned char* p = &data;
    for(size_t i = 0; i < sizeof(T)/2;i++){
        std::swap(p[i],p[sizeof(T)-i-1]);
    }
}
}

std::optional<WSFrame> WS_read_frame(Blob & blob){
    WSFrameHead125 frame_head;
    if(blob.read(&frame_head,sizeof(WSFrameHead125),0)!=sizeof(WSFrameHead125)) return {};
    WSFrame frame;
    frame.FIN = frame_head.FIN;
    frame.opcode = frame_head.opcode;
    frame.mask = frame_head.mask;
    unsigned long long offset_pos = sizeof(WSFrameHead125);
    if(frame_head.payload_len==126){
        unsigned int payload_len;
        if(blob.read(&payload_len,sizeof(payload_len),offset_pos)!= sizeof(payload_len))return {};
        frame.payload_len = payload_len;
        reverse_binary(frame.payload_len);
        offset_pos+=sizeof(payload_len);
    }else if(frame_head.payload_len==127){
        unsigned long long payload_len;
        if(blob.read(&payload_len,sizeof(payload_len),offset_pos)!= sizeof(payload_len))return {};
        frame.payload_len = payload_len;
        reverse_binary(frame.payload_len);
        offset_pos+=sizeof(payload_len);
    }else{
        frame.payload_len = frame_head.payload_len;
    }
    if(frame.masking_key){// read mask
        if(blob.read(&frame.masking_key,sizeof(frame.masking_key),offset_pos)!= sizeof(frame.masking_key))return {};
        offset_pos+=sizeof(frame.masking_key);
    }
    char* temp = (char*)malloc(frame.payload_len);
    frame.payload = Blob::take_over(temp);
    if(blob.read(temp,frame.payload_len,offset_pos)!=frame.payload_len) return {};
    blob.pop_front(offset_pos+frame.payload_len);
    return std::move(frame);
}

Blob WS_to_blob(WSFrame const& frame){
    Blob blob;
    if(frame.payload_len<126){
        WSFrameHead125 frame_head;
        frame_head.FIN = frame.FIN;
        frame_head.opcode = frame.opcode;
        frame_head.mask = frame.mask;
        frame_head.payload_len = frame.payload_len;
        blob << frame_head;
        if(frame.mask) blob << frame.masking_key;
        blob.assign(frame.payload.data(),frame.payload_len,0);
    }
    else if(frame.payload_len<=std::numeric_limits<unsigned int>::max()){
        WSFrameHead126 frame_head;
        frame_head.FIN = frame.FIN;
        frame_head.opcode = frame.opcode;
        frame_head.mask = frame.mask;
        frame_head.payload_len = 126;
        frame_head.payload_len_ex = frame.payload_len;
        blob << frame_head;
        if(frame.mask) blob << frame.masking_key;
        blob.assign(frame.payload.data(),frame.payload_len,0);
    }
    else{
        WSFrameHead127 frame_head;
        frame_head.FIN = frame.FIN;
        frame_head.opcode = frame.opcode;
        frame_head.mask = frame.mask;
        frame_head.payload_len = 127;
        frame_head.payload_len_ex = frame.payload_len;
        blob << frame_head;
        if(frame.mask) blob << frame.masking_key;
        blob.assign(frame.payload.data(),frame.payload_len,0);
    }
    return std::move(blob);
}

}

  /*
  0               1               2               3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-------+-+-------------+-------------------------------+
  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
  |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
  | |1|2|3|       |K|             |                               |
  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
  |     Extended payload length continued, if payload len == 127  |
  + - - - - - - - - - - - - - - - +-------------------------------+
  |                               |Masking-key, if MASK set to 1  |
  +-------------------------------+-------------------------------+
  | Masking-key (continued)       |          Payload Data         |
  +-------------------------------- - - - - - - - - - - - - - - - +
  :                     Payload Data continued ...                :
  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
  |                     Payload Data continued ...                |
  +---------------------------------------------------------------+
  opcode:
  *  %x0 denotes a continuation frame
  *  %x1 denotes a text frame
  *  %x2 denotes a binary frame
  *  %x3-7 are reserved for further non-control frames
  *  %x8 denotes a connection close
  *  %x9 denotes a ping
  *  %xA denotes a pong
  *  %xB-F are reserved for further control frames
  Payload length:  7 bits, 7+16 bits, or 7+64 bits
  Masking-key:  0 or 4 bytes
  */

 // FIN : 0: the message is not end; 1: the message is end
 // RCV1 / 2 / 3 : extension, normaly be 0
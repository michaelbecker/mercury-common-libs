/**
 *  The bit and structure definitions for Segments.
 */
#ifndef SEGMENT_BITS_H__
#define SEGMENT_BITS_H__

#ifdef __cplusplus
extern "C" {
#endif


#pragma pack(1)

typedef struct SegmentBits_t_{

    char SegmentTag[4];         //  Should always be 'SGMT'
    unsigned int Id;            //  Actual segment identifier
    unsigned int UniqueTag;     //  Id of segment in list, for modify.
    unsigned char Data[1];      //  Optional data.

}SegmentBits_t;

#pragma pack()

#ifdef __cplusplus
}
#endif


#endif


#ifndef COMMON_SEGMENTS_H__
#define COMMON_SEGMENTS_H__


#ifdef __cplusplus
extern "C" {
#endif


enum CommonSegments {
    
    CsRepeat                = 0x30000,
};



#pragma pack(1)

typedef struct CsRepeatData_t_ {

    unsigned int Index;     /**< Where to branch to */
    unsigned int Count;     /**< How many times to loop */

}CsRepeatData_t;



#pragma pack()

#ifdef __cplusplus
}
#endif



#endif


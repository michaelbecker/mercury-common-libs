#ifndef I_SEGMENT_ABORT_H__
#define I_SEGMENT_ABORT_H__



#include "SegmentExecutionEngine.h"


//
//  We use a Virtual Static Idiom to handle wrapping the 
//  callback functions. 
//  
//  http://c2.com/cgi/wiki?VirtualStaticIdiom
//
class ISegmentAbort {

    //
    //  ---------------------------------------------------------------------
    //
    public:
        //  
        //  Constructor
        //
        ISegmentAbort(ENGINE_HANDLE hEngine,       //  Which Engine you are tying this to?
                       unsigned int SubCommand,     //  The ACTION Subcommand.
                       unsigned int UserFlags,      //  Are you using user flags?
                       unsigned int DataLength,     //  The expected length of your data.
                       char *Name
                       );


    //
    //  ---------------------------------------------------------------------
    //
    protected:
        //
        //  true => the next segment will be started.
        //  false => the next segment will be skipped.
        //
        virtual bool StartNextSegment(unsigned char *Data) = 0;

        //
        //  true => the next segment will continue.
        //  false => the next segment will be skipped.
        //
        virtual bool ContinueNextSegment(unsigned char *Data) = 0;


        virtual void ToStringCallback(unsigned char *Data,
                                      char *StringBuffer,
                                      int *MaxStringBufferLength) = 0;


        virtual SeStatus ValidateCallback(unsigned char *Data) = 0;


        virtual ~ISegmentAbort();

        unsigned int GetDataLength();

        const char *GetName();

    //
    //  ---------------------------------------------------------------------
    //
    private:

        ENGINE_HANDLE Engine;
        unsigned int SubCommand;
        unsigned int UserFlags;
        unsigned int DataLength;
        char *Name;


        static SeStatus 
            RouteStart(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
            unsigned char *Data,
            void *UserData,
            void *Extension);

        static SeStatus 
            RouteRun(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
            unsigned char *Data,
            void *UserData,
            void *Extension);

        static SeStatus 
            RouteValidate(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
            unsigned char *Data,
            void *UserData);

        static void 
            RouteToString(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
            unsigned char *Data,
            void *UserData,
            void *Extension,
            char *StringBuffer,
            int *MaxStringBufferLength);


        /**
        *  Private copy constructors.
        *  This is strictly a utility class.
        */
        ISegmentAbort(const ISegmentAbort &);
        ISegmentAbort & operator=(const ISegmentAbort &);
};


#endif 

#ifndef I_SEGMENT_REPEAT_UNTIL_H__
#define I_SEGMENT_REPEAT_UNTIL_H__


#include "SegmentExecutionEngine.h"


//
//  We use a Virtual Static Idiom to handle wrapping the 
//  callback functions. 
//  
//  http://c2.com/cgi/wiki?VirtualStaticIdiom
//
class ISegmentRepeatUntil {

    //
    //  ---------------------------------------------------------------------
    //
    public:
        //  
        //  Constructor
        //
        ISegmentRepeatUntil(ENGINE_HANDLE hEngine,       //  Which Engine you are tying this to?
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
        //  You must call BranchNeeded() or BranchNotNeeded() from here.
        //
        virtual void StartCallback(unsigned char *Data) = 0;

        virtual void ToStringCallback(unsigned char *Data,
                                      char *StringBuffer,
                                      int *MaxStringBufferLength) = 0;

        virtual SeStatus ValidateCallback(unsigned char *Data) = 0;


        virtual ~ISegmentRepeatUntil();

        //
        //  If you decide you need to branch, call this in your
        //  StartCallback().
        //
        void BranchNeeded(int index);

        //
        //  If you do not need to branch, call this in your 
        //  StartCallback().
        //
        void BranchNotNeeded();

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
        int NewIndex;
        bool DoBranch;
        char *Name;


        static SeStatus 
            RouteStart(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
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
        ISegmentRepeatUntil(const ISegmentRepeatUntil &);
        ISegmentRepeatUntil & operator=(const ISegmentRepeatUntil &);
};


#endif 

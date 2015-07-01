#ifndef I_SEGMENT_SIMPLE_H__
#define I_SEGMENT_SIMPLE_H__


#include "SegmentExecutionEngine.h"


//
//  We use a Virtual Static Idiom to handle wrapping the 
//  callback functions. 
//  
//  http://c2.com/cgi/wiki?VirtualStaticIdiom
//
class ISegmentSimple {

    //
    //  ---------------------------------------------------------------------
    //
    public:
        //  
        //  Constructor
        //
        ISegmentSimple(ENGINE_HANDLE hEngine,       //  Which Engine you are tying this to?
                       unsigned int SubCommand,     //  The ACTION Subcommand.
                       unsigned int UserFlags,      //  Are you using user flags?
                       unsigned int DataLength,     //  The expected length of your data.
                       char *Name
                       );


    //
    //  ---------------------------------------------------------------------
    //
    protected:

        virtual SeStatus StartCallback(unsigned char *Data) = 0;

        virtual void ToStringCallback(  unsigned char *Data,
                                        char *StringBuffer,
                                        int *MaxStringBufferLength) = 0;

        virtual SeStatus ValidateCallback(unsigned char *Data) = 0;

        virtual void StopCallback(unsigned char *Data);

        virtual SeStatus RunCallback(unsigned char *Data);

        virtual void ModifyCallback(unsigned char *Data);

        virtual ~ISegmentSimple();

        unsigned int GetDataLength();

        const char *GetName();

        unsigned int GetSubcommand();

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

        static void 
        RouteStop(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                    unsigned char *Data,
                    void *UserData,
                    void *Extension);

        static SeStatus 
        RouteRun(SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                    unsigned char *Data,
                    void *UserData,
                    void *Extension);

        static void 
        RouteModify(   SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                        unsigned char *Data,
                        void *UserData,
                        void *Extension);

        static SeStatus 
        RouteValidate(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                        unsigned char *Data,
                        void *UserData);

        static void 
        RouteToString(  SE_SEGMENT_LIST_HANDLE SegmentListHandle,
                        unsigned char *Data,
                        void *UserData,
                        void *Extension,
                        char *StringBuffer,
                        int *MaxStringBufferLength);


        /**
        *  Private copy constructors.
        *  This is strictly a utility class.
        */
        ISegmentSimple(const ISegmentSimple &);
        ISegmentSimple & operator=(const ISegmentSimple &);
};


#endif 

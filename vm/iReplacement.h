#ifndef I_REPLACEMENT_H
#define I_REPLACEMENT_H

class IReplacement{
    public:
        virtual ~IReplacement(){}
        virtual void pageUsed( int pPage ) = 0;
        virtual int getReplacement() = 0;
        virtual void clearPage( int pPage ) = 0;
        virtual void markPage( int pPage ) = 0;
};

#endif

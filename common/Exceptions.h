#ifndef EXCEPTIONS_INCLUDED
#define EXCEPTIONS_INCLUDED

class ShaderMissingException
{
public:
    ShaderMissingException(const std::string &str) {};
};

class OutofBoundsException
{
public:
    OutofBoundsException(int w, int h) {}

};

#endif
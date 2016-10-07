/**
 *  TempDir.h
 * 
 *  Class that holds the name of the temp dir
 * 
 *  @copyright 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
class TempDir
{
private:
    /**
     *  Value of the tempdir
     *  @var std::string
     */
    std::string _value;
    
public:
    /**
     *  Constructor
     */
    TempDir() : _value(Php::ini_get("yothalot.temp-directory").stringValue())
    {
        // check validity of the temp dir
        struct stat info;

        // check if the temp dir is accesible and it's really a folder
        if (stat(_value.data(), &info) == 0 && S_ISDIR(info.st_mode)) return;

        // the temp dir is invalid, use the default temporary directory
        _value = "/tmp";
    }
    
    /**
     *  Destructor
     */
    virtual ~TempDir() = default;
    
    /**
     *  Cast to std::string
     *  @return std::string
     */
    operator const std::string& () const { return _value; }
    
    /**
     *  Cast to a const char *
     *  @return const char *
     */
    operator const char * () const { return _value.data(); }
};

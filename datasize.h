/**
 *  DataSize.h
 * 
 *  Class that is used to convert data sizes (like '4kb' or '12mb') into
 *  actual bytes and the other way round
 * 
 *  @copyright 2008 - 2016 Copernica BV
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <sstream>
 
/**
 *  Class definition
 */
class DataSize
{
    /**
     *  The number of bytes
     *  @var size_t
     */
    size_t _bytes;

public:
    /**
     *  Constructor
     *  @param  bytes   The number of bytes
     *  @param  size    Human readable representation of the size
     */
    DataSize(const std::string &size) : _bytes(0)
    {
        // use iostream to parse the size
        std::istringstream str(size);
        
        // get the number of bytes and the size-name
        float bytes; std::string sizename;
        str >> bytes;
        str >> sizename;
        
        // check name of the size
        if (strcasecmp(sizename.c_str(), "kb") == 0) _bytes = bytes * 1024;
        if (strcasecmp(sizename.c_str(), "mb") == 0) _bytes = bytes * 1024 * 1024;
        if (strcasecmp(sizename.c_str(), "gb") == 0) _bytes = bytes * 1024 * 1024 * 1024;
    }
    
    /**
     *  Destructor
     */
    virtual ~DataSize() = default;
    
    /**
     *  Cast to size_t
     *  @return size_t
     */
    operator size_t() const { return _bytes; }
};

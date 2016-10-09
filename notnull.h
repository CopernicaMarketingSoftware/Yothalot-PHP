/**
 *  NotNull.h
 *
 *  Utility class that is used to prevent null values. It throws an exception
 *  whan a null value is passed
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2016 Copernica bV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
template <typename TYPE>
class NotNull
{
private:
    /**
     *  The actual value
     *  @var TYPE
     */
    TYPE *_value;

public:
    /**
     *  Constructor
     *  @param  value
     *  @throws std::runtime_error
     */
    NotNull(TYPE *value) : _value(value)
    {
        // all is ok if indeed not null
        if (_value != nullptr) return;
        
        // report error
        throw std::runtime_error("unexpected null value");
    }
    
    /**
     *  Destructor
     */
    virtual ~NotNull() = default;
    
    /**
     *  Cast to the original pointer
     *  @return TYPE
     */
    operator TYPE* () const
    {
        // expose value
        return _value;
    }
};


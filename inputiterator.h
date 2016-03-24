/**
 *  InputIterator.php
 *
 *  Class for iterating over an Yothalot\Input object
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "record.h"
#include <phpcpp.h>

/**
 *  Class definition
 */
class InputIterator : public Php::Iterator
{
private:
    /**
     *  Name of the input file
     *  @var std::string
     */
    std::string _name;

    /**
     *  Run in strict mode (with data integrity checks)
     *  @var bool
     */
    bool _strict;

    /**
     *  The input object that is being iterated over
     *  @var std::shared_ptr<Yothalot::Input>
     */
    std::shared_ptr<Yothalot::Input> _input;

    /**
     *  The current key
     *  @return int64_t
     */
    int64_t _key = 0;

    /**
     *  The current value
     *  @var std::unique_ptr<Yothalot::Record>
     */
    std::shared_ptr<Yothalot::Record> _current;

public:
    /**
     *  Constructor
     *  @param  base        The original object
     *  @param  name        Name of the object
     *  @param  strict      Run in strict mode
     */
    InputIterator(Php::Base *base, const std::string &name, bool strict) :
        Php::Iterator(base), _name(name), _strict(strict) {}

    /**
     *  Destructor
     */
    virtual ~InputIterator() {}

    /**
     *  Is the iterator on a valid position
     *  @return bool
     */
    virtual bool valid() override
    {
        // do we have a current?
        return _input != nullptr && _current != nullptr;
    }

    /**
     *  The value at the current position
     *  @return Php::Value
     */
    virtual Php::Value current() override
    {
        // return object
        return Php::Object("Yothalot\\Record", new Record(_current));
    }

    /**
     *  The key at the current position
     *  @return Value
     */
    virtual Php::Value key() override
    {
        return _key;
    }

    /**
     *  Move to the next position
     */
    virtual void next() override
    {
        // not possible when there is no input
        if (!_input) return;

        // prevent exceptions
        try
        {
            // create a new record
            _current = std::make_shared<Yothalot::Record>(*_input);

            // update key
            ++_key;
        }
        catch (...)
        {
            // object is in an invalid state now
            _current = nullptr;
        }
    }

    /**
     *  Rewind the iterator to the front position
     */
    virtual void rewind() override
    {
        // key is back
        _key = 0;

        // prevent exceptions
        try
        {
            // reconstruct input
            _input = std::make_shared<Yothalot::Input>(_name.data(), _strict);
            
            // read first record
            _current = std::make_shared<Yothalot::Record>(*_input);
        }
        catch (...)
        {
            // error state
            _input = nullptr;
            _current = nullptr;
        }
    }
};


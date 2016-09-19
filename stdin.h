/**
 *  Stdin.h
 *
 *  Input that is read from stdin, and that contains the data for the
 *  task, and the original serialized object
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 - 2016 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include "revived.h"

/**
 *  Class definition
 */
class Stdin
{
private:
    /**
     *  The revived data
     *  @var Revived
     */
    std::unique_ptr<Revived> _data;

public:
    /**
     *  Constructor that reads input and turns it into a string
     */
    Stdin()
    {
        // see http://stackoverflow.com/questions/201992/how-to-read-until-eof-from-cin-in-c
        // don't skip the whitespace while reading
        std::cin >> std::noskipws;

        // use stream iterators to copy the stream to a string
        std::istream_iterator<char> it(std::cin);
        std::istream_iterator<char> end;

        // create the string
        _data.reset(new Revived(std::move(std::string(it, end))));
    }
    
    /**
     *  Destructor
     */
    virtual ~Stdin() = default;
    
    /**
     *  The user-supplied PHP object
     *  @return Php::Value
     */
    const Php::Value &object() const
    {
        return _data->object();
    }
    
    /**
     *  The input data
     *  @return const char *
     */
    const char *data() const
    {
        return _data->data();
    }
    
    /**
     *  Size of the data
     *  @return size_t
     */
    size_t size() const
    {
        return _data->size();
    }
};


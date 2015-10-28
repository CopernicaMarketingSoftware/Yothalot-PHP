/**
 *  Record.h
 * 
 *  Class that wraps a log record for use in Yothalot
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
#include "recorditerator.h"

/**
 *  Class definition
 */
class Record :
    public Php::Base,
    public Php::Countable,
    public Php::ArrayAccess,
    public Php::Traversable
{
private:
    /**
     *  The record being wrapped
     *  @var std::shared_ptr<Yothalot::Record>
     */
    std::shared_ptr<Yothalot::Record> _record;

public:
    /**
     *  Constructor
     *  @param  record
     */
    Record(const std::shared_ptr<Yothalot::Record> &record) : 
        _record(record) {}

    /**
     *  Destructor
     */
    virtual ~Record() {}

    /**
     *  The record identifier
     *  @return Php::Value
     */
    Php::Value identifier() const
    {
        return (int64_t)_record->identifier();
    }

    /**
     *  The record size in bytes
     *  @return Php::Value
     */
    Php::Value size() const
    {
        return (int64_t)_record->bytes();
    }

    /**
     *  Retrieve the number of items in the record
     *  @return long
     */
    virtual long count() override
    {
        return _record->size();
    }

    /**
     *  Retrieve the number of fields in the record
     *  @return Php::Value
     */
    Php::Value fields() const
    {
        return (int64_t)_record->size();
    }

    /**
     *  Convert the record to an array
     *  @return Php::Value
     */
    Php::Value array() const
    {
        // result value
        Php::Array result;

        int i = 0;
        // iterate over all values
        for (auto iter = _record->begin(); iter != _record->end(); ++iter)
        {
            // check type
            if      (iter->isNull())   result.set(i, nullptr);
            else if (iter->isNumber()) result.set(i, iter->number());
            else if (iter->isString()) result.set(i, iter->string());

            ++i;
        }

        // done
        return result;
    }

    /**
     *  Check if a member is set
     *  @param  key
     *  @return bool
     */
    virtual bool offsetExists(const Php::Value &key) override
    {
        // convert to int
        int index = key;

        // check if withing range
        return index >= 0 && index < (int)_record->size();
    }

    /**
     *  Set a member
     *  @param  key
     *  @param  value
     */
    virtual void offsetSet(const Php::Value &key, const Php::Value &value) override
    {
        // not possible
        Php::error << "Impossible to set Yothalot\\Record fields" << std::flush;
    }

    /**
     *  Retrieve a member
     *  @param  key
     *  @return value
     */
    virtual Php::Value offsetGet(const Php::Value &key) override
    {
        // convert to int
        int index = key;

        // check type
        if (_record->isNull(index)) return nullptr;
        if (_record->isNumber(index)) return _record->number(index);
        if (_record->isString(index)) return _record->string(index);

        // not possible
        return nullptr;
    }

    /**
     *  Remove a member
     *  @param key
     */
    virtual void offsetUnset(const Php::Value &key) override
    {
        // not possible
        Php::error << "Impossible to unset Yothalot\\Record fields" << std::flush;
    }

    /**
     *  Get the iterator to traverse the array
     *  @return Iterator
     */
    virtual Php::Iterator *getIterator() override
    {
        return new RecordIterator(this, _record);
    }
};



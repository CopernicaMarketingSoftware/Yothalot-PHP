/**
 *  RecordIterator.h
 *
 *  Class to iterate over a record
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Class definition
 */
class RecordIterator : public Php::Iterator
{
private:
    /**
     *  The record that is being iterated
     *  @var std::shared_ptr<Yothalot::Record> _record;
     */
    std::shared_ptr<Yothalot::Record> _record;

    /**
     *  The current index
     *  @var int
     */
    int _current;

public:
    /**
     *  Constructor
     *  @param  base
     *  @param  record
     */
    RecordIterator(Php::Base *base, const std::shared_ptr<Yothalot::Record> &record) :
        Php::Iterator(base), _record(record), _current(0) {}

    /**
     *  Destructor
     */
    virtual ~RecordIterator() {}

    /**
     *  Is the iterator on a valid position
     *  @return bool
     */
    virtual bool valid() override
    {
        return _current < (int)_record->size();
    }

    /**
     *  The value at the current position
     *  @return Value
     */
    virtual Php::Value current() override
    {
        // check type
        if (_record->isNull(_current)) return nullptr;
        if (_record->isNumber(_current)) return _record->number(_current);
        if (_record->isString(_current)) return _record->string(_current);

        // not possible
        return nullptr;
    }

    /**
     *  The key at the current position
     *  @return Value
     */
    virtual Php::Value key() override
    {
        return _current;
    }

    /**
     *  Move to the next position
     */
    virtual void next() override
    {
        ++_current;
    }

    /**
     *  Rewind the iterator to the front position
     */
    virtual void rewind() override
    {
        _current = 0;
    }
};


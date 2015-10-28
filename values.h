/**
 *  Values.h
 *
 *  The values class.
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include <yothalot.h>

#include "valuesiterator.h"

/**
 *  Class definition
 */
class Values :
    public Yothalot::Values,
    public Php::Base,
    public Php::Traversable {
public:
    /**
     *  Constructor
     */
    Values(const Yothalot::Values &values) : Yothalot::Values(values) {}

    /**
     *  Destructor
     */
    virtual ~Values() {};

    /**
     *  Get the iterator
     *  @return Php::Iterator
     */
    virtual Php::Iterator *getIterator() override
    {
        return new ValuesIterator(this);
    }
};
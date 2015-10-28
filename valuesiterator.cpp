/**
 *  ValuesIterator.cpp
 *
 *  Implementation for the valuesiterator class, simply because
 *  of a circular reference (iterator needs the base).
 *
 *  @author Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include <phpcpp.h>
#include <yothalot.h>
#include "valuesiterator.h"
#include "values.h"

/**
 *  Constructor
 *  @param  values        Object that is being iterated
 */
ValuesIterator::ValuesIterator(Values *values) :
    Php::Iterator(values),
    _values(*values)
{}

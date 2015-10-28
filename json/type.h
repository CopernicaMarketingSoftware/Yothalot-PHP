/**
 *  Type.h
 *
 *  Enum describing the different JSON value types
 *
 *  @author Toon Schoenmakers <toon.schoemakers@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Include guard
 */
#pragma once

/**
 *  Set up namespace
 */
namespace JSON {

/**
 *  Enum describing the different kinds of values
 *  a JSON object can hold
 */
enum class Type : uint8_t
{
    Null,
    Boolean,
    Decimal,
    Integer,
    Object,
    Array,
    String,
    Undefined
};

/**
 *  End of namespace
 */
}

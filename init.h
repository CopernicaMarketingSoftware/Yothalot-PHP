/**
 *  Init.h
 *
 *  The init function that should be called directly on cli.
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

/**
 *  Our global init method, mostly used to call directly from cli using something
 *  like `php -r "YothalotInit('mapper');"`
 *  @param  params
 *  @return Php::Value  Return values are like normal programs really, 0 if success
 *                      something else otherwise.
 */
Php::Value yothalotInit(Php::Parameters &params);
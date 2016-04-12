/**
 *  Base.cpp
 *
 *  Implementation file for retrieving GlusterFS mount point
 *
 *  @author Emiel Bruijntjes <emiel.bruijntjes@copernica.com>
 *  @copyright 2015 Copernica BV
 */

/**
 *  Dependencies
 */
#include "base.h"
#include <phpcpp.h>

/**
 *  Function returns the base directory
 *  @return Yothalot::Base
 */
const Yothalot::Base &base()
{
    // a missing gluster mount may result in an exception
    try
    {
        // create static instance
        static Yothalot::Base instance(Php::ini_get("yothalot.base-directory"));

        // return it
        return instance;
    }
    catch (const std::runtime_error &error)
    {
        // pass the exception on to PHP
        throw Php::Exception(error.what());
    }
}

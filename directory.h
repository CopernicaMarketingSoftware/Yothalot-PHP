/**
 *  Directory.h
 *
 *  Creates a temporary directory in a folder where the Yothalot data
 *  files are going to be stored
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
#include "base.h"
#include <string>
#include <phpcpp.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <dirent.h>

/**
 *  Class definition
 */
class Directory
{
private:
    /**
     *  The full name of the directory
     *  @var    Yothalot::Fullname
     */
    Yothalot::Fullname _name;

public:
    /**
     *  Constructor, will create a temporary directory in the gluster mount point/tmp
     */
    Directory() : _name(base(), "tmp/" + Php::call("uniqid", getpid()).stringValue())
    {
        // try to construct the directory
        if (mkdir(_name.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) return;

        // directory could not be created, make sure base exists
        Yothalot::Fullname basedir(base(), "tmp");

        // create the base directory (we don't check the result, it can also fail
        // because it already exists, and we do not want to check all such details)
        mkdir(basedir.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // retry to create the actual directory
        if (mkdir(_name.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) return;

        // failed to create temp directory twice: it is not our day
        throw std::runtime_error("failed to create temporary directory");
    }

    /**
     *  Constructor on string, that will use temporary directory in the gluster mount point/tmp
     *  @param  name        name of the directory (should not be null)
     */
    Directory(const char *name) : _name(base(), name)
    {
        // if string is not empty check if dir is valid
        DIR *dir = opendir(_name.full());
        if (!dir) throw std::runtime_error("failed to open directory");

        // dir is ok
        closedir(dir);
    }

    /**
     *  Destructor, cleans up the temporary directory.
     */
    virtual ~Directory() {}

    /**
     *  Get to the full path
     *  @return const char *
     */
    const char *full() const
    {
        return _name.full();
    }

    /**
     *  Get the relative path, relative to the gluster mount point.
     *  @return const std::string
     */
    const char *relative() const
    {
        // delegate
        return _name.relative();
    }
};


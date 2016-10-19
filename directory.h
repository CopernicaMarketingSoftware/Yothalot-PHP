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
    
    /**
     *  Does the directory exist
     *  @return bool
     */
    mutable bool _exists = false;


public:
    /**
     *  Constructor, will create a temporary directory in the gluster mount point/tmp
     */
    Directory() : 
        _name(base(), std::string("tmp/") + (std::string)Yothalot::UniqueName()) {}

    /**
     *  Constructor on string, that will use temporary directory in the gluster mount point/tmp
     *  @param  name        name of the directory, relative to the glusterfs mount point (should not be null)
     */
    Directory(const char *name) : _name(base(), name) {}

    /**
     *  Destructor, can be empty since the server is cleaning up this directory
     */
    virtual ~Directory() = default;
    
    /**
     *  Cast to boolean: does the directory exist?
     *  @return bool
     */
    operator bool () const { return exists(); }
    bool operator! () const { return !exists(); }
    
    /**
     *  Does the directory exist?
     *  @return bool
     */
    bool exists() const
    {
        // do we already know this?
        if (_exists) return true;
        
        // file properties
        struct stat props;
        
        // read in properties
        if (stat(_name.full(), &props) != 0) return false;
        
        // must be a dir
        return _exists = S_ISDIR(props.st_mode);
    }

    /**
     *  Create the directory
     *  @return bool
     */
    bool create()
    {
        // if the directory already exists
        if (exists()) return true;
        
        // try to construct the directory
        if (mkdir(_name.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0) return true;

        // directory could not be created, make sure base exists
        Yothalot::Fullname basedir(base(), "tmp");

        // create the base directory (we don't check the result, it can also fail
        // because it already exists, and we do not want to check all such details)
        mkdir(basedir.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

        // retry to create the actual directory
        return _exists = mkdir(_name.full(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
    }

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
    
    /**
     *  Traverse the directory and apply a callback to each item
     *  passing the raw entry information
     *  it resets at the end so it can be called again.
     *  @param  callback
     *  @return bool
     */
    bool traverse(const std::function<void(struct dirent *entry)> &callback) const
    {
        // reset the dir so traverse can be called again.
        auto *dirp = opendir(_name);

        // leap out if directory could not be opened
        if (dirp == nullptr) return false;
        
        // iterate over the files and stop if all files are consumed
        while (true)
        {
            // read next entry
            auto *result = readdir(dirp);

            // If all files are consumed we break
            if (result == nullptr) break;

            // we ignore the '.' and '..' files
            if (strcmp(result->d_name, ".") == 0 || strcmp(result->d_name, "..") == 0) continue;

            // Callback
            callback(result);
        }

        // close dir
        closedir(dirp);
        
        // done
        return true;
    }

    /**
     *  Traverse the directory and apply a callback to each item
     *  passing the entry name as a string
     *  it resets at the end so it can be called again.
     *  @param  callback
     *  @return bool
     */
    bool traverse(const std::function<void(const char *name)> &callback) const
    {
        // call the other traverse implementation
        return traverse([callback](struct dirent *entry) {
            callback(entry->d_name);
        });
    }
    
    /**
     *  Remove the directory (and all files in it)
     *  @return true
     */
    bool remove()
    {
        // traverse over the files
        traverse([this](const char *name) {
            
            // construct full path name
            std::string fullname(_name.full());
            
            // append file name
            fullname.append("/").append(name);
            
            // remove it
            unlink(fullname.data());
        });
        
        // remove the directory
        return rmdir(full()) == 0;
    }
};


<?php
/**
 *  Example implementation of a MapReduce algorithm
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 *  @documentation private
 */

class MyRacer implements Yothalot\Racer
{
    private $output;

    public function __construct($output)
    {
        // parse the output as path name (just in case someone called
        // the constructor with an absolute path instead of a relative path)
        $path = new Yothalot\Path($output);

        // store
        $this->output = $path->relative();
    }

    public function includes()
    {
        // we only have to include this file
        return array(__FILE__);
    }

    /**
     *  We'll emit some really basic info in case the data is 3..
     */
    function process($data)
    {
        if ($data == 3) return array("key1" => $data, "key2" => $this->output, "key3" => $data, "cwd" => getcwd());
    }
}

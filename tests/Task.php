<?php
/**
 *  Example implementation of a MapReduce algorithm
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 *  @documentation private
 */

class Task implements Yothalot\Task
{
    /**
     *  Simply store this variable
     */
    private $output;

    public function __construct($output)
    {
        // store the input
        $this->output = $output;
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
        return array("key1" => "one", "key2" => $this->output, "key3" => "three", "cwd" => getcwd());
    }
}

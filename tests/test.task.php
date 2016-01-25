<?php
/**
 *  Script to test
 *
 *  @author    Toon Schoenmakers <toon.schoenmakers@copernica.com>
 *  @copyright 2015 Copernica BV
 *  @documentation private
 */

/**
 *  Dependencies
 */
require_once('Task.php');

// Create an instance of the WordCount algorithm
$test = new Task("Input");

// create the connection
$master = new Yothalot\Connection(array("host" => "localhost", "vhost" => "gluster"));

// create the new job
$job = new Yothalot\Job($master, $test);

// start the job and wait for the result
$result = $job->wait();

// simply var_dump the result
var_dump($result->result());


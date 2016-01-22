<?php
/**
 *  Example implementation of a MapReduce algorithm
 *
 *  @copyright 2015 Copernica BV
 *  @documentation private
 */

/**
 *  WordCount.php
 *
 *  This is a serializable class - which means that it can be serialized by the
 *  Yothalot framework, and transferred to other nodes in the Yothalot cluster.
 *  It is therefore possible that the map(), reduce() and write() methods will
 *  all be called on different nodes in the cluster. It is the responsibility of
 *  the Yothalot framework to make calls to your object at the right time. You
 *  are not supposed to make calls to methods of this class yourself.
 */
class KvChainedLineCount implements Yothalot\MapReduce2
{
    /**
     *  The output file
     *  @var string
     */
    private $output;

    /**
     *  Constructor
     *  @param  string      File to which output should be written
     */
    public function __construct($output)
    {
        // store
        $this->output = $output;
    }

    /**
     *  The Yothalot framework serializes and unserializes objects and transfers
     *  them between nodes, so that the algorithm can run close to the files
     *  that are being mapped and reduced.
     *
     *  If there are PHP files that have to be loaded before an object is
     *  unserialized, you should implement this method to return the names of
     *  these files.
     *
     *  Of course, you must make sure that the files returned by this method are
     *  accessible on all the servers that are in the Yothalot cluster. You can
     *  for example achieve this by simply storing your PHP files on the
     *  distributed GlusterFS file system.
     *
     *  @return string[]    Array of to-be-included files
     */
    public function includes()
    {
        // we only have to include this file
        return array(__FILE__);
    }

    /**
     *  The mapper algorithm starts by calling the map() function in your class
     *  for every input value that you send to your mapper.
     *
     *  In this WordCount example the input value is a string with a file name.
     *  This name is relative to the GlusterFS mount point.
     *
     *  @param  mixed       Value that is being mapped (in this example: a path)
     *  @param  Reducer     Reducer object to which we may emit key/value pairs
     */
    function map($key, $value, Yothalot\Reducer $reducer)
    {
        // the value is a filename that we can open
        if (!is_resource($fp = fopen($value, "r"))) throw new Exception("Unable to open ".$value);

        // read one line at a time (this implementation is scalable, only one
        // line is being read, so that the script never has to use a lot of
        // memory to load the entire file)
        while (($line = fgets($fp)) !== false) $reducer->emit($value, 1);

        // close the file
        fclose($fp);
    }

    /**
     *  When the mapper algorithm emits identical keys, the Yothalot framework
     *  will start making calls to the reduce() method to reduce the values
     *  linked to these keys. This reduced value should be passed to the writer.
     *
     *  It is very well possible that the reduce() method gets called more than
     *  once for the same key (for example if so many keys were found that
     *  multiple reducers were started). The value that you emit might therefore
     *  be an intermediate value that is going to be reduced for a second or
     *  third time because it is finally written.
     *
     *  In this specific WordCount implementation, the key is a word, and
     *  values is a list of numbers telling how often the word was found.
     *
     *  @param  mixed       The key for which values should be reduced
     *  @param  Values      Traversable object with values linked to the key
     *  @param  Writer      Object to which the reduced value can be sent
     */
    function reduce($key, Yothalot\Values $values, Yothalot\Writer $writer)
    {
        // total number of occurances for the word
        $total = 0;

        // iterate over the found values (the Yothalot\Values class is a
        // traversable class, over which you can iterate).
        foreach ($values as $value) $total += $value;

        // emit the reduced value to the writer
        $writer->emit($total);
    }

    /**
     *  The final step in the reducer process calls the write() method once for
     *  every found key, and for each reduced value.
     *
     *  In this specific WordCount example, the key is a word, and the
     *  value the total number of occurances
     *
     *  @param  mixed       The key for which the result comes in
     *  @param  mixed       Fully reduced value
     */
    function write($key, $value)
    {
        // the output file is stored on the gluster
        $path = new Yothalot\Path($this->output);

        // open an output file
        $output = new Yothalot\Output($path->absolute());

        // add the key and value
        $output->kv($key, $value);
    }
}

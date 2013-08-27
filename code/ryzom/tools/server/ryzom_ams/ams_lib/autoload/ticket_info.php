<?php

class Ticket_Log{
    
    private $tInfoId;
    private $ticket;
    
    private $shardid;
    private $user_position;
    private $view_position;
    private $client_version;
    private $patch_version;
    private $server_tick;
    private $connect_state;
    private $local_address;    
    private $memory;
    private $os;
    private $processor;  
    private $cpu_id;
    private $cpu_mask;
    private $ht;
    private $nel3d;

    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
   
    
    //Creates a log entry
    public static function createTicketInfo($info_array) {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_log (Timestamp, Query, Ticket, Author) VALUES (now(), :query, :ticket, :author )";
        $values = Array('ticket' => $ticket_id, 'author' => $author_id, 'query' => json_encode(array($action,$arg)));
        $dbl->execute($query, $values);
    }


    //return constructed element based on TLogId
    public static function constr_TInfoId( $id) {
        $instance = new self();
        $instance->setTInfoId($id);
        return $instance;
    }
     
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setTInfoId($values['TInfoId']);
        $this->setTicket($values['Ticket']);
        $this->setShardId($values['ShardId']);
        $this->setUser_Position($values['UserPosition']);
        $this->setView_Position($values['ViewPosition']);
        
        $this->setClient_Version($values['ClientVersion']);
        $this->setPatch_Version($values['PatchVersion']);
        $this->setServer_Tick($values['ServerTick']);
        $this->setConnect_State($values['ConnectState']);
        $this->setLocal_Address($values['LocalAddress']);
        
        $this->setMemory($values['Memory']);
        $this->setOS($values['OS']);
        $this->setProcessor($values['Processor']);
        $this->setCPUId($values['CPUID']);
        $this->setCPU_Mask($values['CpuMask']);
        $this->setHT($values['HT']);
        $this->setNel3D($values['NeL3D']);
    } 

    //Load with tInfoId
    public function load_With_TInfoId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_info WHERE TInfoId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    
    //Load with ticket Id
    public function load_With_TId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_info WHERE Ticket=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->set($row);
    }
    

    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getTInfoId(){
        return $this->tInfoId;
    }
    
    public function getTicket(){
        return $this->ticket;
    }
    
    public function getShardId(){
        return $this->shardid;
    }
    
    public function getUser_Position(){
        return $this->user_position;
    }
   
    public function getView_Position(){
        return $this->view_position;
    }
    
    public function getClient_Version(){
        return $this->client_version;
    }
    
    public function getPatch_Version(){
       return $this->patch_version;
    }
    
    public function getServer_Tick(){
        return $this->server_tick;
    }
    
    public function getConnect_State(){
        return $this->connect_state;
    }
   
    public function getLocal_Address(){
        return $this->local_address;
    }
    
    public function getMemory(){
        return $this->memory;
    }
    
    public function getOS(){
       return $this->os;
    }
    
    public function getProcessor(){
        return $this->processor;
    }
    
    
    public function getCPUId(){
        return $this->cpu_id;
    }
    
    public function getCPU_Mask(){
       return $this->cpu_mask;
    }
    
    public function getHT(){
       return $this->ht;
    }
    
    public function getNel3D(){
        return $this->nel3d;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    public function setTInfoId($id){
        $this->tInfoId = $id;
    }   
        
    public function setTicket($t){
        $this->ticket = $t;
    }
    
    public function setShardId($s){
        $this->shardid = $s;
    }
    
    public function setUser_Position($u){
        $this->user_position = $u;
    }
   
    public function setView_Position($v){
        $this->view_position = $v;
    }
    
    public function setClient_Version($c){
        $this->client_version = $c;
    }
    
    public function setPatch_Version($p){
       $this->patch_version = $p;
    }
    
    public function setServer_Tick($s){
        $this->server_tick = $s;
    }
    
    public function setConnect_State($c){
        $this->connect_state = $c;
    }
   
    public function setLocal_Address($l){
        $this->local_address = $l;
    }
    
    public function setMemory($m){
        $this->memory = $m;
    }
    
    public function setOS($o){
       $this->os = $o;
    }
    
    public function setProcessor($p){
        $this->processor = $p;
    }
    
    
    public function setCPUId($c){
        $this->cpu_id = $c;
    }
    
    public function setCPU_Mask($c){
       $this->cpu_mask = $c;
    }
    
    public function setHT($h){
       $this->ht = $h;
    }
    
    public function setNel3D($n){
        $this->nel3d = $n;
    }
    
}
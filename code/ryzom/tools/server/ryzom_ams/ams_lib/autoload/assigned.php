<?php

class Assigned{
    
    private $user;
    private $ticket;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
  
    //Assigns a ticket to a user or returns error message
    public static function AssignTicket( $user_id, $ticket_id) {
        $dbl = new DBLayer("lib");
        //check if ticket is already assigned, if so return "ALREADY ASSIGNED"
        if(! Assigned::isAssigned($ticket_id)){
            $assignation = new Assigned();
            $assignation->set(array('User' => $user_id, 'Ticket' => $ticket_id));
            $assignation->create();
            return "SUCCESS";
        }else{
            return "ALREADY_ASSIGNED";
        }
      
    }
    
    public static function isAssigned( $ticket_id ) {
        $dbl = new DBLayer("lib");
        //check if ticket is already assigned
        if(  $dbl->execute(" SELECT * FROM `assigned` WHERE `Ticket` = :ticket_id", array('ticket_id' => $ticket_id) )->rowCount() ){
            return true;
        }else{
            return false;
        } 
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
     
    public function __construct() {
    }
    
    //set values
    public function set($values) {
        $this->setUser($values['User']);
        $this->setGroup($values['Ticket']);
    }
    
    public function create() {
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO `assigned' (`User`,`Ticket`) VALUES (:user, :ticket)";
        $values = Array('user' => $this->getUser(), 'ticket' => $this->getTicket());
        $dbl->execute($query, $values);
    }
    
    //delete entry
    public function delete() {
        $dbl = new DBLayer("lib");
        $query = "DELETE FROM `assigned` WHERE `User` = :user_id and `Ticket` = :ticket_id";
        $values = array('user_id' => $this->getUser() ,'ticket_id' => $this->getGroup());
        $dbl->execute($query, $values);
    }

    //Load with sGroupId
    public function load( $user_id, $user_id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM `assigned` WHERE `Ticket` = :ticket_id AND `User` = :user_id", Array('ticket_id' => $ticket_id, 'user_id' => $user_id));
        $row = $statement->fetch();
        $this->set($row);
    }
    

    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getUser(){
        return $this->user;
    }
    
    public function getTicket(){
        return $this->ticket;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////

    public function setUser($u){
        $this->user = $u;
    }
    
    public function setTicket($g){
        $this->ticket = $g;
    }
   
    
}
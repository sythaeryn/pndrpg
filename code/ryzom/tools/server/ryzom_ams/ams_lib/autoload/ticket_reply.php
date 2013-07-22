<?php

class Ticket_Reply{
    private $tReplyId;
    private $ticket;
    private $content;
    private $author;
    private $timestamp;
    private $hidden;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    //return constructed element based on TCategoryId
    public static function constr_TReplyId( $id) {
        $instance = new self();
        $instance->setTReplyId($id);
        return $instance;
    }
    
    
    //return constructed element based on TCategoryId
    public static function getRepliesOfTicket( $ticket_id, $view_as_admin) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_reply INNER JOIN ticket_content INNER JOIN ticket_user ON ticket_reply.Content = ticket_content.TContentId and ticket_reply.Ticket=:id and ticket_user.TUserId = ticket_reply.Author ORDER BY ticket_reply.TReplyId ASC", array('id' => $ticket_id));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $tReply){
            if(! $tReply['Hidden'] || $view_as_admin){
                $instanceAuthor = Ticket_User::constr_TUserId($tReply['Author']);
                $instanceAuthor->setExternId($tReply['ExternId']);
                $instanceAuthor->setPermission($tReply['Permission']);
                
                $instanceContent = new Ticket_Content();
                $instanceContent->setTContentId($tReply['TContentId']);
                $instanceContent->setContent($tReply['Content']);
                
                $instanceReply = new self();
                $instanceReply->setTReplyId($tReply['TReplyId']);
                $instanceReply->setTimestamp($tReply['Timestamp']);
                $instanceReply->setAuthor($instanceAuthor);
                $instanceReply->setTicket($ticket_id);
                $instanceReply->setContent($instanceContent);
                $instanceReply->setHidden($tReply['Hidden']);
                $result[] = $instanceReply;
            }
        }
        return $result; 
    }
    
    public static function createReply($content, $author, $ticket_id , $hidden, $ticket_creator){
        $ticket_content = new Ticket_Content();
        $ticket_content->setContent($content);
        $ticket_content->create();
        $content_id = $ticket_content->getTContentId();
 
        $ticket_reply = new Ticket_Reply();
        $ticket_reply->set(Array('Ticket' => $ticket_id,'Content' => $content_id,'Author' => $author, 'Hidden' => $hidden));
        $ticket_reply->create();
        $reply_id = $ticket_reply->getTReplyId();
        
        if($ticket_creator == $author){
            Ticket::updateTicketStatus( $ticket_id, 1, $author);
        }
 
        Ticket_Log::createLogEntry( $ticket_id, $author, 4, $reply_id);
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    
    public function __construct() {
    }


    //Set ticket_reply object
    public function set($values){
        $this->setTicket($values['Ticket']);
        $this->setContent($values['Content']);
        $this->setAuthor($values['Author']);
        if(isset($values['Timestamp'])){
            $this->setTimestamp($values['Timestamp']);
        }
        if(isset($values['Hidden'])){
            $this->setHidden($values['Hidden']);
        }
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer("lib");
        $query = "INSERT INTO ticket_reply (Ticket, Content, Author, Timestamp, Hidden) VALUES (:ticket, :content, :author, now(), :hidden)";
        $values = Array('ticket' => $this->ticket, 'content' => $this->content, 'author' => $this->author, 'hidden' => $this->hidden);
        $this->tReplyId = $dbl->executeReturnId($query, $values); 
    }

    //return constructed element based on TId
    public function load_With_TReplyId( $id) {
        $dbl = new DBLayer("lib");
        $statement = $dbl->execute("SELECT * FROM ticket_reply WHERE TReplyId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tReplyId = $row['TReplyId'];
        $this->ticket = $row['Ticket'];
        $this->content = $row['Content'];
        $this->author = $row['Author'];
        $this->timestamp = $row['Timestamp'];
        $this->hidden = $row['Hidden'];
    }
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer("lib");
        $query = "UPDATE ticket SET Ticket = :ticket, Content = :content, Author = :author, Timestamp = :timestamp, Hidden = :hidden WHERE TReplyId=:id";
        $values = Array('id' => $this->tReplyId, 'timestamp' => $this->timestamp, 'ticket' => $this->ticket, 'content' => $this->content, 'author' => $this->author, 'hidden' => $this->hidden);
        $statement = $dbl->execute($query, $values);
    }
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
     
    public function getTicket(){
        return $this->ticket;
    }
   
   
    public function getContent(){
        return $this->content;
    }
    
    public function getAuthor(){
        return $this->author;
    }
    
    public function getTimestamp(){
        return Helpers::outputTime($this->timestamp);
    }
    
    
    public function getTReplyId(){
        return $this->tReplyId;
    }
   
    public function getHidden(){
        return $this->hidden;
    } 
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
     
    public function setTicket($t){
        $this->ticket = $t;
    }
   
   
    public function setContent($c){
        $this->content = $c;
    }
    
    public function setAuthor($a){
        $this->author =  $a;
    }
    
    public function setTimestamp($t){
        $this->timestamp = $t;
    }
    
    
    public function setTReplyId($i){
        $this->tReplyId = $i;
    }
        
    public function setHidden($h){
        $this->hidden = $h;
    }
}
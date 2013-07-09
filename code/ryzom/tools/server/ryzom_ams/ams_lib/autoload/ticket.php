<?php

class Ticket{
    private $tId;
    private $timestamp;
    private $title;
    private $status;
    private $queue;
    private $ticket_category;
    private $author;
    private $db;
    
    ////////////////////////////////////////////Functions////////////////////////////////////////////////////
    
    /*FUNCTION: getTicketTitlesOf()
     * return all ticket of the given author's id.
     *
     */
    public static function getTicketsOf($author, $db_data) {
        $dbl = new DBLayer($db_data);
        $statement = $dbl->execute("SELECT * FROM ticket INNER JOIN ticket_user ON ticket.Author = ticket_user.TUserId and ticket_user.ExternId=:id", array('id' => $author));
        $row = $statement->fetchAll();
        $result = Array();
        foreach($row as $ticket){
            $instance = new self($db_data);
            $instance->setTId($ticket['TId']);
            $instance->setTimestamp($ticket['Timestamp']);
            $instance->setTitle($ticket['Title']);
            $instance->setStatus($ticket['Status']);
            $instance->setQueue($ticket['Queue']);
            $instance->setTicket_Category($ticket['Ticket_Category']);
            $instance->setAuthor($ticket['Author']);
            $result[] = $instance;
        }
        return $result; 
    }
    
    
    /*FUNCTION: create_Ticket()
     * creates a ticket + first initial reply and fills in the content of it!
     *
     */
    public static function create_Ticket( $title, $content, $category, $author, $db_data) {
        
        $ticket = new Ticket($db_data);
        $ticket->set($title,0,0,$category,$author);
        $ticket->create();
        $ticket_id = $ticket->getTId();
        
 
        $ticket_content = new Ticket_Content($db_data);
        $ticket_content->setContent($content);
        $ticket_content->create();
        $content_id = $ticket_content->getTContentId();
 
        
        $ticket_reply = new Ticket_Reply($db_data);
        $ticket_reply->set($ticket_id, $content_id, $author);
        $ticket_reply->create();
        
    }
    
    ////////////////////////////////////////////Methods////////////////////////////////////////////////////
    public function __construct($db_data) {
        $this->db = $db_data;
    }


    //Set ticket object
    public function set($t,$s,$q,$t_c,$a){
        $this->title = $t;
        $this->status = $s;
        $this->queue = $q;
        $this->ticket_category = $t_c;
        $this->author = $a;
    }
    
    //create ticket by writing private data to DB.
    public function create(){
        $dbl = new DBLayer($this->db);
        $query = "INSERT INTO ticket (Timestamp, Title, Status, Queue, Ticket_Category, Author) VALUES (now(), :title, :status, :queue, :tcat, :author)";
        $values = Array('title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author);
        $this->tId = $dbl->executeReturnId($query, $values); ;
    }

    //return constructed element based on TId
    public function load_With_TId( $id) {
        $dbl = new DBLayer($this->db);
        $statement = $dbl->execute("SELECT * FROM ticket WHERE TId=:id", array('id' => $id));
        $row = $statement->fetch();
        $this->tId = $row['TId'];
        $this->timestamp = $row['Timestamp'];
        $this->title = $row['Title'];
        $this->status = $row['Status'];
        $this->queue = $row['Queue'];
        $this->ticket_category = $row['Ticket_Category'];
        $this->author = $row['Author'];
    }
    
    
    //update private data to DB.
    public function update(){
        $dbl = new DBLayer($this->db);
        $query = "UPDATE ticket SET Timestamp = :timestamp, Title = :title, Status = :status, Queue = :queue, Ticket_Category = :tcat, Author = :author WHERE TId=:id";
        $values = Array('id' => $this->tId, 'timestamp' => $this->timestamp, 'title' => $this->title, 'status' => $this->status, 'queue' => $this->queue, 'tcat' => $this->ticket_category, 'author' => $this->author);
        $statement = $dbl->execute($query, $values);
    }
    
    
    ////////////////////////////////////////////Getters////////////////////////////////////////////////////
    
    public function getTId(){
        return $this->tId;
    }
    
    public function getTimestamp(){
        return $this->timestamp;
    }
    
    public function getTitle(){
        return $this->title;
    }
    
    public function getStatus(){
        return $this->status;
    }
    
    public function getStatusText(){
        $statusId = $this->getStatus();
        if ($statusId == 0){
            return "Waiting on support..";
        }else if($statusId == 1){
            return "Being handled..";
        }else if($statusId == 2){
            return "Closed";
        }
        return "Error";
    }
    
    public function getCategoryName(){
        global $cfg;
        $category = Ticket_Category::constr_TCategoryId($this->getTicket_Category(), $cfg['db']['lib']);
        return $category->getName();  
    }
    
    public function getQueue(){
        return $this->queue;
    }
    
    public function getTicket_Category(){
        return $this->ticket_category;
    }
    
    public function getAuthor(){
        return $this->author;
    }
    
    ////////////////////////////////////////////Setters////////////////////////////////////////////////////
    
    public function setTId($id){
        $this->tId = $id;
    }
    
    public function setTimestamp($ts){
        $this->timestamp = $ts;
    }
    
    public function setTitle($t){
        $this->title = $t;
    }
    
    public function setStatus($s){
        $this->status = $s;
    }
    
    public function setQueue($q){
        $this->queue = $q;
    }
    
    public function setTicket_Category($tc){
        $this->ticket_category = $tc;
    }
    
    public function setAuthor($a){
        $this->author = $a;
    }
    
}
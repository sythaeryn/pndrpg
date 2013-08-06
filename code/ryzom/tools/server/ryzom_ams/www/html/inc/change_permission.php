<?php

function change_permission(){
    
      //if logged in
    if(WebUsers::isLoggedIn()){
       
        if(ticket_user::isAdmin($_SESSION['ticket_user'])){
            
            if(isset($_GET['user_id']) && isset($_GET['value']) && $_GET['user_id'] != 1 && $_GET['value'] < 4 ){
                $user_id = filter_var($_GET['user_id'], FILTER_SANITIZE_NUMBER_INT);
                $value = filter_var($_GET['value'], FILTER_SANITIZE_NUMBER_INT);
                
                Ticket_User::change_permission(Ticket_User::constr_ExternId($user_id)->getTUserId(), $value);
                header("Location: index.php?page=show_user&id=".$user_id);
                exit;
              
                
            }else{
                //ERROR: GET PARAMS not given or trying to change admin
                header("Location: index.php?page=show_user&id=".$user_id);
                exit;
            }
        
        }else{
            //ERROR: No access!
            $_SESSION['error_code'] = "403";
            header("Location: index.php?page=error");
            exit;
     
        }
    
    }else{
        //ERROR: not logged in!
        header("Location: index.php");
        exit;
    }
    
    
}
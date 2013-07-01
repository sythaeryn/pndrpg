<?php

function settings(){
    if(WebUsers::isLoggedIn()){
            //in case id-GET param set it's value as target_id, if no id-param is given, ue the session id.
            if(isset($_GET['id'])){
                if(WebUsers::isAdmin() && ($_GET['id']!= $_SESSION['id'])){
                    $result['isAdmin'] = "TRUE";
                }
                $result['target_id'] = $_GET['id'];
            }else{
                $result['target_id'] = $_SESSION['id'];
            }

            return $result;
    }else{
        //ERROR: not logged in!
        print("not logged in!");
        exit;
    }
}
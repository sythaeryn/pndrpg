<?php

function add_sgroup(){
    global $INGAME_WEBPATH;
    global $WEBPATH;
    if(WebUsers::isLoggedIn()){
        
        if( Ticket_User::isAdmin(unserialize($_SESSION['ticket_user']))){
            $name = filter_var($_POST['Name'],FILTER_SANITIZE_STRING);
            $inner_tag = filter_var($_POST['Tag'], FILTER_SANITIZE_STRING);
            $tag = "[" . $inner_tag . "]";
            $inner_tag = filter_var($_POST['Tag'], FILTER_SANITIZE_STRING);
            $groupemail = filter_var($_POST['GroupEmail'], FILTER_SANITIZE_STRING);
            $imap_mailserver = filter_var($_POST['IMAP_MailServer'], FILTER_SANITIZE_STRING);
            $imap_username = filter_var($_POST['IMAP_Username'], FILTER_SANITIZE_STRING);
            $imap_password = filter_var($_POST['IMAP_Password'], FILTER_SANITIZE_STRING);
            
            $result['RESULT_OF_ADDING'] = Support_Group::createSupportGroup($name, $tag, $groupemail, $imap_mailserver, $imap_username, $imap_password);
            $result['permission'] = unserialize($_SESSION['ticket_user'])->getPermission();
            $result['no_visible_elements'] = 'FALSE';
            $result['username'] = $_SESSION['user'];
            //global $SITEBASE;
            //require($SITEBASE . '/inc/sgroup_list.php');
            //$result= array_merge($result, sgroup_list());
            //return helpers :: loadtemplate( 'sgroup_list', $result, true);
            if (Helpers::check_if_game_client()) {
                header("Location: ".$INGAME_WEBPATH."?page=sgroup_list");
            }else{
                header("Location: ".$WEBPATH."?page=sgroup_list");
            }
            exit;
            
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
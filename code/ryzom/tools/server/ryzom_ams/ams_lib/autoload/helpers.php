<?php

class Helpers{
     

     public static function loadTemplate( $template, $vars = array (), $returnHTML = false )
    {
         global $AMS_LIB;
         global $SITEBASE;
         global $AMS_TRANS;
         global $INGAME_LAYOUT;
         //define('SMARTY_SPL_AUTOLOAD',1);
         require_once $AMS_LIB . '/smarty/libs/Smarty.class.php';
         spl_autoload_register('__autoload');

         $smarty = new Smarty;
         $smarty->setCompileDir($SITEBASE.'/templates_c/');
         $smarty->setCacheDir($SITEBASE.'/cache/');
         $smarty -> setConfigDir($SITEBASE . '/configs/' );
         // turn smarty debugging on/off
         $smarty -> debugging = false;
         // caching must be disabled for multi-language support
         $smarty -> caching = false;
         $smarty -> cache_lifetime = 120;

         helpers :: create_folders ();
          global $FORCE_INGAME;
          if ( helpers::check_if_game_client() or $FORCE_INGAME ){
             $smarty -> template_dir = $AMS_LIB . '/ingame_templates/';
             $smarty -> setConfigDir( $AMS_LIB . '/configs' );
             $variables = parse_ini_file( $AMS_LIB . '/configs/ingame_layout.ini', true );
             foreach ( $variables[$INGAME_LAYOUT] as $key => $value ){
               $smarty -> assign( $key, $value );
             }
          }else{
             $smarty -> template_dir = $SITEBASE . '/templates/';
             $smarty -> setConfigDir( $SITEBASE . '/configs' );
          }

          foreach ( $vars as $key => $value ){
             $smarty -> assign( $key, $value );
             }
             
      
         $variables = Helpers::handle_language();
         foreach ( $variables[$template] as $key => $value ){
             $smarty -> assign( $key, $value );
             }

          if( isset($vars['permission']) && $vars['permission'] == 3 ){
               $inherited = "extends:layout_admin.tpl|";
          }else if( isset($vars['permission']) && $vars['permission'] == 2){
               $inherited = "extends:layout_mod.tpl|";
          }else if( isset($vars['permission']) && $vars['permission'] == 1){
               $inherited = "extends:layout_user.tpl|";
          }else{
               $inherited ="";
          }

          
         
          if($returnHTML == true){
               return $smarty ->fetch($inherited . $template . '.tpl' );
          }else{
               $smarty -> display( $inherited . $template . '.tpl' );
          }
     }

     static public function create_folders(){
         global $AMS_LIB;
         global $SITEBASE;
         $arr = array( $AMS_LIB . '/ingame_templates/',
             $AMS_LIB . '/configs',
             //$AMS_LIB . '/cache',
             $SITEBASE . '/cache/',
             $SITEBASE . '/templates/',
             $SITEBASE . '/templates_c/',
             $SITEBASE . '/configs'
             );
         foreach ( $arr as & $value ){
             if ( !file_exists( $value ) ){
                 //echo $value;
                 print($value);
                 mkdir($value);
                 }
             }

         }

     static public function check_if_game_client()
    {
         // if HTTP_USER_AGENT is not set then its ryzom core
          global $FORCE_INGAME;
          if (( strpos($_SERVER['HTTP_USER_AGENT'],"Ryzom") === 0) || $FORCE_INGAME){
             return true;
          }else{
             return false;
             }
     }
       
     static public function handle_language(){
          global $DEFAULT_LANGUAGE;
          global $AMS_TRANS;
          
          //if user wants to change the language
          if(isset($_GET['Language']) && isset($_GET['setLang'])){
               //The ingame client sometimes sends full words, derive those!
               switch($_GET['Language']){
                    
                    case "English":
                         $lang = "en";
                         break;
                    
                    case "French":
                         $lang = "fr";
                         break;
                    
                    default:
                         $lang = $_GET['Language'];           
               }
               //if the file exists en the setLang = true
               if( file_exists( $AMS_TRANS . '/' . $lang . '.ini' ) && $_GET['setLang'] == "true"){
                    //set a cookie & session var and incase logged in write it to the db!
                    setcookie( 'Language', $lang , time() + 60*60*24*30 );
                    $_SESSION['Language'] = $lang;
                    if(WebUsers::isLoggedIn()){
                         WebUsers::setLanguage($_SESSION['id'],$lang);
                    }     
               }else{
                    $_SESSION['Language'] = $DEFAULT_LANGUAGE;
               }
          }else{
               //if the session var is not set yet
               if(!isset($_SESSION['Language'])){
                    //check if a cookie already exists for it
                    if ( isset( $_COOKIE['Language'] ) ) { 
                         $_SESSION['Language'] = $_COOKIE['Language'];
                    //else use the default language
                    }else{
                         $_SESSION['Language'] = $DEFAULT_LANGUAGE;
                    }
               }
          }
          
          if ($_SESSION['Language'] == ""){
               $_SESSION['Language'] = $DEFAULT_LANGUAGE;
          }
          return parse_ini_file( $AMS_TRANS . '/' .  $_SESSION['Language'] . '.ini', true );
          
     }
     
     
     //Time output function for handling the time display function.
     static public function outputTime($time, $str = 1){
          global $TIME_FORMAT;
          if($str){
               return date($TIME_FORMAT,strtotime($time));
          }else{
               return date($TIME_FORMAT,$time);
          }
     }
     
     static public function  check_login_ingame(){
          if ( helpers :: check_if_game_client () or $forcelibrender = false ){
               $dbr = new DBLayer("ring");
               if (isset($_GET['UserId']) && isset($_COOKIE['ryzomId'])){
                    $id = $_GET['UserId'];
                    $statement = $dbr->execute("SELECT * FROM ring_users WHERE user_id=:id AND cookie =:cookie", array('id' => $id, 'cookie' => $_COOKIE['ryzomId']));
                    if ($statement->rowCount() ){
                         $entry = $statement->fetch();
			//print_r($entry);
                         return array('id' => $entry['user_id'], 'name' => $entry['user_name']); 
                    }else{
                         return "FALSE";
                    }
               }else{
                    return "FALSE";
               }
          }else{
               return "FALSE";
          }
     }
}
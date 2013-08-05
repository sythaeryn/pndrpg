<?php
class Users{
     
     /**
     * Function check_register
     *
     * @takes $array with username,password and email
     * @return string Info: Returns a string, if input data is valid then "success" is returned, else an array with errors
     */ 
     public function check_Register($values){
          // check values
          if ( isset( $values["Username"] ) and isset( $values["Password"] ) and isset( $values["ConfirmPass"] ) and isset( $values["Email"] ) ){
               $user = Users::checkUser( $values["Username"] );
               $pass = Users::checkPassword( $values["Password"] );
               $cpass = Users::confirmPassword($pass,$values["Password"],$values["ConfirmPass"]);
               $email = Users::checkEmail( $values["Email"] );
          }else{
               $user = "";
               $pass = "";
               $cpass = "";
               $email = "";
          }

          if ( ( $user == "success" ) and ( $pass == "success" ) and ( $cpass == "success" ) and ( $email == "success" ) and (  isset( $_POST["TaC"] ) ) ){
               return "success";
          }else{
               $pageElements = array(
               //'GAME_NAME' => $GAME_NAME,
               // 'WELCOME_MESSAGE' => $WELCOME_MESSAGE,
                'USERNAME' => $user,
                'PASSWORD' => $pass,
                'CPASSWORD' => $cpass,
                'EMAIL' => $email
                );
               if ( $user != "success" ){
                    $pageElements['USERNAME_ERROR'] = 'TRUE';
               }else{
                    $pageElements['USERNAME_ERROR'] = 'FALSE';
               }
       
               if ( $pass != "success" ){
                    $pageElements['PASSWORD_ERROR'] = 'TRUE';
               }else{
                    $pageElements['PASSWORD_ERROR'] = 'FALSE';
               }
               if ( $cpass != "success" ){
                    $pageElements['CPASSWORD_ERROR'] = 'TRUE';
               }else{
                    $pageElements['CPASSWORD_ERROR'] = 'FALSE';
               }
               if ( $email != "success" ){
                    $pageElements['EMAIL_ERROR'] = 'TRUE';
               }else{
                    $pageElements['EMAIL_ERROR'] = 'FALSE';
               }
               if ( isset( $_POST["TaC"] ) ){
                    $pageElements['TAC_ERROR'] = 'FALSE';
               }else{
                    $pageElements['TAC_ERROR'] = 'TRUE';
               }
               return $pageElements;
          }

     }
     
     


    /**
     * Function checkUser
     *
     * @takes $username
     * @return string Info: Returns a string based on if the username is valid, if valid then "success" is returned
     */
     private function checkUser( $username )
     {
          if ( isset( $username ) ){
               if ( strlen( $username ) > 12 ){
                    return "Username must be no more than 12 characters.";
               }else if ( strlen( $username ) < 5 ){
                    return "Username must be 5 or more characters.";
               }else if ( !preg_match( '/^[a-z0-9\.]*$/', $username ) ){
                    return "Username can only contain numbers and letters.";
               }else if ( $username == "" ){
                    return "You have to fill in a username";
               }elseif ($this->checkUserNameExists($username)){
                    return "Username " . $username . " is in use.";
               }else{
                    return "success";
               }
          }
          return "fail";
     }
         
     /**
     * Function checkUserNameExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the www db.
     */
     protected function checkUserNameExists($username){
          //You should overwrite this method with your own version!
          print('this is the base class!');
          
     }
         
         
    /**
     * Function checkPassword
     *
     * @takes $pass
     * @return string Info: Returns a string based on if the password is valid, if valid then "success" is returned
     */
     private function checkPassword( $pass )
    {
         if ( isset( $pass ) ){
             if ( strlen( $pass ) > 20 ){
                 return "Password must be no more than 20 characters.";
                 }elseif ( strlen( $pass ) < 5 ){
                 return "Password must be more than 5 characters.";
                 }elseif ( $pass == ""){
                 return "You have to fill in a password";
                 }else{
                 return "success";
                 }
             }
         return "fail";
         }
         
         
    /**
     * Function confirmPassword
     *
     * @takes $pass
     * @return string Info: Verify's $_POST["Password"] is the same as $_POST["ConfirmPass"]
     */
     private function confirmPassword($pass_result,$pass,$confirmpass)
    {
          if ($confirmpass==""){
             return "You have to fill in the confirmation password.";
          }
          else if ( ( $pass ) != ( $confirmpass ) ){
             return "Passwords do not match.";
          }else if($pass_result != "success"){
               return;
          }else{
             return "success";
          }
          return "fail";
     }
     
     
    /**
     * Function checkEmail
     *
     * @takes $email
     * @return
     */
     public function checkEmail( $email )
    {
         if ( isset( $email ) ){
               if ( !Users::validEmail( $email ) ){
                    return "Email address is not valid.";
               }else if($email == ""){
                    return "You have to fill in an email address";
               }else if ($this->checkEmailExists($email)){
                    return "Email is in use.";
               }else{
                    return "success";
               }
          }
          return "fail";
     }


     /**
     * Function checkEmailExists
     *
     * @takes $username
     * @return string Info: Returns true or false if the user is in the www db.
     */
     protected function checkEmailExists($email){
          //TODO: You should overwrite this method with your own version!
          print('this is the base class!');
          
     }
         
         
    /**
     * Function validEmail
     *
     * @takes $email
     * @return true or false depending on if its a valid email format.
     */
     private function validEmail( $email ){
          $isValid = true;
          $atIndex = strrpos( $email, "@" );
          if ( is_bool( $atIndex ) && !$atIndex ){
               $isValid = false;
          }else{
               $domain = substr( $email, $atIndex + 1 );
               $local = substr( $email, 0, $atIndex );
               $localLen = strlen( $local );
               $domainLen = strlen( $domain );
               if ( $localLen < 1 || $localLen > 64 ){
                    // local part length exceeded
                    $isValid = false;
               }else if ( $domainLen < 1 || $domainLen > 255 ){
                    // domain part length exceeded
                    $isValid = false;
               }else if ( $local[0] == '.' || $local[$localLen - 1] == '.' ){
                    // local part starts or ends with '.'
                    $isValid = false;
               }else if ( preg_match( '/\\.\\./', $local ) ){
                    // local part has two consecutive dots
                    $isValid = false;
               }else if ( !preg_match( '/^[A-Za-z0-9\\-\\.]+$/', $domain ) ){
                    // character not valid in domain part
                    $isValid = false;
               }else if ( preg_match( '/\\.\\./', $domain ) ){
                    // domain part has two consecutive dots
                    $isValid = false;
               }else if ( !preg_match( '/^(\\\\.|[A-Za-z0-9!#%&`_=\\/$\'*+?^{}|~.-])+$/', str_replace( "\\\\", "", $local ) ) ){
                    // character not valid in local part unless
                    // local part is quoted
                    if ( !preg_match( '/^"(\\\\"|[^"])+"$/', str_replace( "\\\\", "", $local ) ) ){
                         $isValid = false;
                     }
               }
               if ( $isValid && !( checkdnsrr( $domain, "MX" ) || checkdnsrr( $domain, "A" ) ) ){
                    // domain not found in DNS
                    $isValid = false;
               }
          }
          return $isValid;
     }



     /**
     * Function generateSALT
     *
     * @takes $length, which is by default 2
     * @return a random salt of 2 chars
     */
     public static function generateSALT( $length = 2 )
    {
         // start with a blank salt
        $salt = "";
         // define possible characters - any character in this string can be
        // picked for use in the salt, so if you want to put vowels back in
        // or add special characters such as exclamation marks, this is where
        // you should do it
        $possible = "2346789bcdfghjkmnpqrtvwxyzBCDFGHJKLMNPQRTVWXYZ";
         // we refer to the length of $possible a few times, so let's grab it now
        $maxlength = strlen( $possible );
         // check for length overflow and truncate if necessary
        if ( $length > $maxlength ){
             $length = $maxlength;
             }
         // set up a counter for how many characters are in the salt so far
        $i = 0;
         // add random characters to $salt until $length is reached
        while ( $i < $length ){
             // pick a random character from the possible ones
            $char = substr( $possible, mt_rand( 0, $maxlength - 1 ), 1 );
             // have we already used this character in $salt?
            if ( !strstr( $salt, $char ) ){
                 // no, so it's OK to add it onto the end of whatever we've already got...
                $salt .= $char;
                 // ... and increase the counter by one
                $i++;
                 }
             }
         // done!
        return $salt;
     }
     


    /**
     * Function create
     *
     * @takes $array with name,pass and mail
     * @return ok if it's get correctly added to the shard, else return lib offline and put in libDB, if libDB is also offline return liboffline.
     */
     public static function createUser($values, $user_id){     
          try {
               //make connection with and put into shard db
               $dbs = new DBLayer("shard");
               $dbs->execute("INSERT INTO user (Login, Password, Email) VALUES (:name, :pass, :mail)",$values);
               ticket_user::createTicketUser( $user_id, 1);
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer("lib");  
                    $dbl->execute("INSERT INTO ams_querycache (type, query, db) VALUES (:type, :query, :db)",array("type" => "createUser",
                    "query" => json_encode(array($values["name"],$values["pass"],$values["mail"])), "db" => "shard"));
                    ticket_user::createTicketUser( $user_id , 1 );
                    return "shardoffline";
               }catch (PDOException $e) {
                    print_r($e);
                    return "liboffline";
               }
          } 

     }
     
     
     protected function checkLoginMatch($user,$pass){
          print('This is the base class!');
     }
     
     public function check_change_password($values){
          //if admin isn't changing others
          if(!$values['adminChangesOther']){
               if ( isset( $values["user"] ) and isset( $values["CurrentPass"] ) and isset( $values["ConfirmNewPass"] ) and isset( $values["NewPass"] ) ){
                    $match = $this->checkLoginMatch($values["user"],$values["CurrentPass"]);
                    $newpass = $this->checkPassword($values["NewPass"]);
                    $confpass = $this->confirmPassword($newpass,$values["NewPass"],$values["ConfirmNewPass"]);
               }else{
                    $match = "";
                    $newpass = "";
                    $confpass = "";
               }
          }else{
               //if admin is indeed changing someone!
               if ( isset( $values["user"] ) and isset( $values["ConfirmNewPass"] ) and isset( $values["NewPass"] ) ){
                    $newpass = $this->checkPassword($values["NewPass"]);
                    $confpass = $this->confirmPassword($newpass,$values["NewPass"],$values["ConfirmNewPass"]);
               }else{
                    $newpass = "";
                    $confpass = "";
               }
          }
          if (  !$values['adminChangesOther'] and ( $match != "fail" ) and ( $newpass == "success" ) and ( $confpass == "success" ) ){
               return "success";
          }else if($values['adminChangesOther'] and ( $newpass == "success" ) and ( $confpass == "success" ) ){
               return "success";
          }else{
               $pageElements = array(
                'newpass_error_message' => $newpass,
                'confirmnewpass_error_message' => $confpass
                );
               if(!$values['adminChangesOther']){
                    $pageElements['match_error_message'] = $match;
                    if ( $match != "fail" ){
                         $pageElements['MATCH_ERROR'] = 'FALSE';
                    }else{
                         $pageElements['MATCH_ERROR'] = 'TRUE';
                    }
               }
               if ( $newpass != "success" ){
                    $pageElements['NEWPASSWORD_ERROR'] = 'TRUE';
               }else{
                    $pageElements['NEWPASSWORD_ERROR'] = 'FALSE';
               }
               if ( $confpass != "success" ){
                    $pageElements['CNEWPASSWORD_ERROR'] = 'TRUE';
               }else{
                    $pageElements['CNEWPASSWORD_ERROR'] = 'FALSE';
               }
               return $pageElements;
          }
     }
     
     protected function setAmsPassword($user, $pass){
          
           $values = Array('user' => $user, 'pass' => $pass);
           
           try {
               //make connection with and put into shard db
               $dbs = new DBLayer("shard");
               $dbs->execute("UPDATE user SET Password = :pass WHERE Login = :user ",$values);
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer("lib");
                    $dbl->execute("INSERT INTO ams_querycache (type, query, db) VALUES (:type, :query, :db)",array("type" => "change_pass",
                    "query" => json_encode(array($values["user"],$values["pass"])), "db" => "shard"));
                    return "shardoffline";
               }catch (PDOException $e) {
                    return "liboffline";
               }
          } 
     }
     
     protected function setAmsEmail($user, $mail){
          
           $values = Array('user' => $user, 'mail' => $mail);
           
           try {
               //make connection with and put into shard db
               $dbs = new DBLayer("shard");
               $dbs->execute("UPDATE user SET Email = :mail WHERE Login = :user ",$values);
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer("lib");
                    $dbl->execute("INSERT INTO ams_querycache (type, query, db) VALUES (:type, :query, :db)",array("type" => "change_mail",
                    "query" => json_encode(array($values["user"],$values["mail"])), "db" => "shard"));
                    return "shardoffline";
               }catch (PDOException $e) {
                    return "liboffline";
               }
          } 
     }
}


     
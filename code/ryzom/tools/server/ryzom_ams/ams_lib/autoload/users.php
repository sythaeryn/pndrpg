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
          if ( isset( $values["Username"] ) and isset( $values["Password"] ) and isset( $values["Email"] ) ){
               $user = Users :: checkUser( $values["Username"] );
               $pass = Users :: checkPassword( $values["Password"] );
               $cpass = Users :: confirmPassword($pass);
               $email = Users :: checkEmail( $values["Email"] );
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
     public function checkUser( $username )
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
               /*}elseif ($this->dbs->execute("SELECT * FROM user WHERE Login = :name",array('name' => $username))->rowCount()){
                    return "Username " . $username . " is in use.";*/
               }else{
                    return "success";
               }
          }
          return "fail";
     }
         
         
    /**
     * Function checkPassword
     *
     * @takes $pass
     * @return string Info: Returns a string based on if the password is valid, if valid then "success" is returned
     */
     public function checkPassword( $pass )
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
     public function confirmPassword($pass_result)
    {
          if ( ( $_POST["Password"] ) != ( $_POST["ConfirmPass"] ) ){
             return "Passwords do not match.";
          }else if ($_POST["ConfirmPass"]==""){
             return "You have to fill in the confirmation password.";
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
               /*}elseif ( $this->dbs->execute("SELECT * FROM user WHERE Email = :email",array('email' => $email))->rowCount()){
                    return "Email is in use.";*/}
               else{
                    return "success";
               }
          }
          return "fail";
     }



    /**
     * Function validEmail
     *
     * @takes $email
     * @return true or false depending on if its a valid email format.
     */
     public function validEmail( $email ){
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
     public function generateSALT( $length = 2 )
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
     function createUser($values){     
          try {
               //make connection with and put into shard db
               global $cfg;
               $dbs = new DBLayer($cfg['db']['shard']);  
               $dbs->execute("INSERT INTO user (Login, Password, Email) VALUES (:name, :pass, :mail)",$values["params"]);
               return "ok";
          }
          catch (PDOException $e) {
               //oh noooz, the shard is offline! Put in query queue at ams_lib db!
               try {
                    $dbl = new DBLayer($cfg['db']['lib']);  
                    $dbl->execute("INSERT INTO ams_querycache (type, query) VALUES (:type, :query)",array("type" => "createUser",
                    "query" => json_encode(array($values["params"]["name"],$values["params"]["pass"],$values["params"]["mail"]))));
                    return "shardoffline";
               }catch (PDOException $e) {
                    print_r($e);
                    return "liboffline";
               }
          } 

     }
     
}



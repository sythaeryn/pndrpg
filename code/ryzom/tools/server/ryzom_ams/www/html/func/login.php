<?php
/**
* This function is beign used to login a user.
* It will first check if the sent POST data returns a match with the DB, if it does, some session variables will be appointed to the user and he will be redirected to the index page again.
* If it didn't match, the template will be reloaded and a matching error message will be shown.
* @author Daan Janssens, mentored by Matthew Lagoe
*/
function login(){
	global $INGAME_WEBPATH;
	global $WEBPATH;
	try{
		$login_value = filter_var($_POST['LoginValue'],FILTER_SANITIZE_STRING);
		$password = filter_var($_POST['Password'],FILTER_SANITIZE_STRING);

		//check login type if email or username
		$login_type = WebUsers::checkLoginType($login_value);

		//check if the filtered sent POST data returns a match with the DB

		if($login_type == 'Login')
		{
			$result = WebUsers::checkLoginMatch($login_value, $password);
		}else
		{
			$result = WebUsers::checkLoginMatchUsingEmail($login_value, $password);
		}
	
		if( $result != "fail"){
			//handle successful login

			if($login_type == 'Login')
			{
				$_SESSION['user'] = $login_value;
				$_SESSION['id'] = WebUsers::getId($login_value);
			}else{
				$_SESSION['user'] = WebUsers::getUsernameFromEmail($login_value);
				$_SESSION['id'] = WebUsers::getIdFromEmail($login_value);
			}	

			$_SESSION['ticket_user'] = serialize(Ticket_User::constr_ExternId($_SESSION['id']));
			$user = new WebUsers($_SESSION['id']);
			$_SESSION['Language'] = $user->getLanguage();
			
			$GETString = "";
			foreach($_GET as $key => $value){
				$GETString = $GETString . $key . '=' . $value . "&";
			}		
			if($GETString != ""){
				$GETString = '?'.$GETString;
			}


			//go back to the index page.
			if (Helpers::check_if_game_client()) {
				header( 'Location: '. $INGAME_WEBPATH . $GETString);
			}else{
				header( 'Location: '. $WEBPATH . $GETString);
			}
			exit;
		}else{
			//handle login failure
			$result = Array();
			$result['login_error'] = 'TRUE';
			$result['no_visible_elements'] = 'TRUE';
			helpers :: loadtemplate( 'login', $result);
			exit;
		}	
		
		
	}catch (PDOException $e) {
	     //go to error page or something, because can't access website db
	     print_r($e);
	     exit;
	}
	
}

{extends file="layout.tpl"}
{block name=content}

<div class="row">
	<div class="col-md-12 center login-header">
		<a href="?"><img src="img/mainlogo.png"/></a> 
	</div><!--/span-->
</div><!--/row-->

<div class="row">
	<div class="well col-md-5 center login-box">
		<div class="alert alert-info">
			 {$forgot_password_message}
		</div>
		<form id="signup" class="form-vertical" method="post" action="index.php">
			<legend>{$title}</legend>
			
			<div class="form-group {if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}error{/if}">
			<label class="control-label">{$email_tag}</label>
				<div class="controls">
				    <div class="input-prepend">
					<span class="add-on"><span class="glyphicon glyphicon-envelope"></span></span>
						<input type="text" class="input-xlarge" id="Email" name="Email" placeholder="{$email_default}" {if isset($prevEmail)}value="{$prevEmail}"{/if} rel="popover" data-content="{$email_tooltip}" data-original-title="{$email_default}">
					</div>
				</div>
			</div>
			
			{if isset($EMAIL_ERROR) and $EMAIL_ERROR eq "TRUE"}
			<div class="alert alert-danger">
				<button type="button" class="close" data-dismiss="alert">×</button>
				{$email_doesnt_exist}
			</div>
			{/if}
			{if isset($EMAIL_SUCCESS) and $EMAIL_SUCCESS eq "TRUE"}
			<div class="alert alert-success">
				<button type="button" class="close" data-dismiss="alert">×</button>
				{$email_sent}
			</div>
			{/if}
			<input type="hidden" name="function" value="forgot_password">		
			<p class="center col-md-5">
				<button type="submit" class="btn btn-primary" >Send me the reset link</button>
			</p>
	
		</form>
		<div class="alert alert-info">
			{$register_message} <a href="?page=register">{$here}</a>.<br/> {$login_message} <a href="?page=login">{$here}</a>
		</div>

	</div><!--/span-->
</div><!--/row-->
{/block}

	

{extends file="layout.tpl"}
{block name=content}

<div class="row-fluid">
	<div class="span12 center login-header">
		<img src="img/mainlogo.png"/>
	</div><!--/span-->
</div><!--/row-->

<div class="row-fluid">
				<div class="well span5 center login-box">
					<div class="alert alert-info">
						Please login with your Username and Password.
					</div>
					<form method="post" action="index.php" class="form-horizontal">
						<fieldset>
							<div data-rel="tooltip" class="input-prepend" data-original-title="Username">
								<span class="add-on"><i class="icon-user"></i></span><input type="text" value="" id="username" name="username" class="input-large span10" autofocus="">
							</div>
							<div class="clearfix"></div>

							<div data-rel="tooltip" class="input-prepend" data-original-title="Password">
								<span class="add-on"><i class="icon-lock"></i></span><input type="password" value="" id="password" name="password" class="input-large span10">
							</div>
							<div class="clearfix"></div>

							<div class="input-prepend">
							<label for="remember" class="remember"><div class="checker" id="uniform-remember"><span><input type="checkbox" id="remember" style="opacity: 0;"></span></div>Remember me</label>
							</div>
							<div class="clearfix"></div>

							<p class="center span5">
							<button class="btn btn-primary" type="submit">Login</button>
							</p>
						</fieldset>
					</form>
					<div class="alert alert-info">
					<strong>Register</strong>
						If you dont have an account yet, create one <a href="?page=register">here</a>!
					</div>
				</div><!--/span-->
			</div>
{/block}


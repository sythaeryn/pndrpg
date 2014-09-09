{block name=content}
{if isset($smarty.get.plugin_action) and $smarty.get.plugin_action eq 'get_achievements'}
<div class="row">	
	<div class="box col-md-12">
		<div class="panel panel-default">
			<div class="panel-heading" data-original-title>
				<span class="glyphicon glyphicon-user"></span> Achievements
			</div>
			<div class="panel-body">
			{if isset($hook_info.Achievements.no_char)}<div class="alert alert-danger"><p>{$hook_info.Achievements.no_char}</p></div>{/if}	
				<div class="row">
				{$hook_info.Achievements.char_achievements}							
				</div>
			</div><!--/span-->
		</div><!--/span-->
	</div><!--/row-->
</div><!--/row-->
{else}
<div class="row">	
			<div class="box col-md-4">
				<div class="panel panel-default">
					<div class="panel-heading" data-original-title="">
						<span class="glyphicon glyphicon-th"></span> Select your Character
					</div>
					<div class="panel-body">
						<div class="row">
							<form id="generateKey" class="form-vertical" method="post" action="index.php?page=layout_plugin&&name={$arrkey}&&plugin_action=get_achievements">
							<div class="form-group">
								<div class="form-group">
									<label class="control-label">Character:</label>
									<div class="controls">
									<select name="Character">	
									{foreach from=$hook_info.Achievements.Character item=element}
									<option value="{$element}">{$element}</option>
									{/foreach}
									</select>	
									</div>
								</div>
								<div class="form-group">
									<label class="control-label"></label>
									<div class="controls">
										<button type="submit" name="get_data" value="true" class="btn btn-primary" style="margin-left:5px; margin-top:10px;">Get Achievements</button>
									</div>
								</div>
							</form>		
							</div>                   
						</div>
					</div><!--/span-->
				</div><!--/span-->
			</div><!--/span-->
</div><!--/row-->
{/if}			
{/block}

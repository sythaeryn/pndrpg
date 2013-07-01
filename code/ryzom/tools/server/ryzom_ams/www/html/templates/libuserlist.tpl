{block name=content}
	
	<div class="row-fluid">
		<div class="box span12">
			<div class="box-header well">
				<h2><i class="icon-info-sign"></i>{$libuserlist_title}</h2>
				<div class="box-icon">
					<a href="#" class="btn btn-round" onclick="javascript:show_help('intro');return false;"><i class="icon-info-sign"></i></a>
					<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
					<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
					<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
				</div>
			</div>
			<div class="box-content">
				<center>
				<p>{$libuserlist_info}</p>
				{if $shard eq "online"}
				<div class="alert alert-success">
					<i class="icon-refresh icon-white"></i>{$shard_online}<a href="#" id="sync" onclick="sync()">{$libuserlist_sync}</a>
					<script>
						function sync(){
							xmlhttp=new XMLHttpRequest();
							xmlhttp.open("POST","../../../ams_lib/cron/sync_cron.php",true);
							xmlhttp.send();
						}
					</script>
				</div>
				{else}
				<div class="alert alert-error">
					<strong><i class="icon-refresh icon-white"></i></strong> {$shard_offline}
				</div>
				{/if}
				</center>
				<div class="clearfix"></div>
			</div>
		</div>
	</div>
			<div class="row-fluid sortable">		
				<div class="box span12">
					<div class="box-header well" data-original-title>
						<h2><i class="icon-user"></i> {$members}</h2>
						<div class="box-icon">
							<a href="#" class="btn btn-setting btn-round"><i class="icon-cog"></i></a>
							<a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
							<a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
						</div>
					</div>
					<div class="box-content">
						<table class="table table-striped table-bordered bootstrap-datatable datatable">
						  <thead>
							  <tr>
								  <th>{$id}</th>
								  <th>{$type}</th>
								  <th>{$name}</th>
								  <th>{$email}</th>
								  <th>{$action}</th>
							  </tr>
						  </thead>   
						  <tbody>
							{foreach from=$liblist item=element}
							<tr>
								<td>{$element.id}</td>
								<td class="center">{$element.type}</td>
								<td class="center">{$element.name}</td>
								<td class="center">{$element.mail}</td>
								<td class="center">
									<a class="btn btn-danger" href="index.php?page=libuserlist&action=remove&id={$element.id}"><i class="icon-trash icon-white"></i>Delete</a>
								</td>
								
							</tr>
							{/foreach}
					
						  </tbody>
					  </table>            
					</div>
				</div><!--/span-->
			
			</div><!--/row-->
{/block}


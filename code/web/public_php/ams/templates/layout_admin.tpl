{extends file="layout.tpl"}
{block name=menu}
	{if $permission eq 3}
	<li class="nav-header hidden-tablet">Main</li>
	<li style="margin-left: -2px;"><a class="ajax-link" href="index.php"><span class="glyphicon glyphicon-home"></span><span class="hidden-tablet"> Dashboard</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_user"><span class="glyphicon glyphicon-user"></span><span class="hidden-tablet"> Profile</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=settings"><span class="glyphicon glyphicon-cog"></span><span class="hidden-tablet"> Settings</span></a></li>
		{if isset($hook_info)} {foreach from=$hook_info key=arrkey item=element}{if isset($element['menu_display'])}<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=layout_plugin&name={$arrkey}"><span class="{$element.icon}"></span><span class="hidden-tablet"> {$element['menu_display']}</span></a></li>{/if}{/foreach}{/if} 
	<li class="nav-header hidden-tablet">Admin</li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=userlist"><span class="glyphicon glyphicon-th-list"></span><span class="hidden-tablet"> Users</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=show_queue&get=todo"><span class="glyphicon glyphicon-th-list"></span><span class="hidden-tablet"> Queues</span></a></li>
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=sgroup_list"><span class="glyphicon glyphicon-briefcase"></span><span class="hidden-tablet"> Support Groups</span></a></li>
		{if isset($hook_info)} {foreach from=$hook_info key=arrkey item=element}{if isset($element.admin_menu_display)}<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=layout_plugin&name={$arrkey}"><span class="{$element.icon}"></span><span class="hidden-tablet"> {$element.admin_menu_display}</span></a></li>{/if}{/foreach}{/if} 
        <li class="nav-header hidden-tablet">Actions</li>
	<li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=plugins"><span class="glyphicon glyphicon-th-list"></span><span class="hidden-tablet"> Plugins</span></a></li>  
        <li style="margin-left: -2px;"><a class="ajax-link" href="index.php?page=syncing"><span class="glyphicon glyphicon-th-list"></span><span class="hidden-tablet"> Syncing</span></a></li>
        <li style="margin-left: -2px;"><a href="?page=logout"><span class="glyphicon glyphicon-off"></span><span class="hidden-tablet"> Logout </span></a></li>
	{/if}
{/block}


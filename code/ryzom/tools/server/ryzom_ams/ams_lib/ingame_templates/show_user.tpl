{block name=content}
    <table width="100%"><tr><td>
	<h2><i class="icon-user"></i> Profile of {$target_name}</h2>
	<table cellpadding="7">
	    
		    <tr>
			<td><strong>Email:</strong></td>
			<td>{$mail}</td>                           
		    </tr>
		    
		    <tr>
			<td><strong>Role:</strong></td>
			<td>
			{if $userPermission eq 1}<span class="label label-success">User</span>{/if}
			{if $userPermission eq 2}<span class="label label-warning">Moderator</span>{/if}
			{if $userPermission eq 3}<span class="label label-important">Admin</span>{/if}
			</td>                           
		    </tr>
		    {if $firstName neq ""}
		    <tr>
			<td><strong>Firstname:</strong></td>
			<td>{$firstName}</td>                           
		    </tr>
		    {/if}
		    {if $lastName neq ""}
		    <tr>
			<td><strong>LastName:</strong></td>
			<td>{$lastName}</td>                           
		    </tr>
		    {/if}
		    {if $country neq ""}
		    <tr>
			<td><strong>Country:</strong></td>
			<td>{$country}</td>                           
		    </tr>
		    {/if}
		    {if $gender neq 0}
		    <tr>
			<td><strong>Gender:</strong></td>
			{if $gender eq 1}
			<td><strong>♂</strong></td>
			{else if $gender eq 2}
			<td><strong>♀</strong></td>
			{/if}
		    </tr>
		    {/if}
    
	    </table>
    </td><td>
            <h2><i class="icon-th"></i>Actions</h2><ul>
		    <li><a href="index.php?page=settings&id={$target_id}">Edit User</a></li>
		    <li><a href="index.php?page=createticket&user_id={$target_id}">Send Ticket</a></li>
		    {if isset($isAdmin) and $isAdmin eq 'TRUE' and $target_id neq 1}
			{if $userPermission eq 1}
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=2">Make Moderator</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=3">Make Admin</a></li>
			{else if $userPermission eq 2 }
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=1">Demote to User</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=3">Make Admin</a></li>
			{else if $userPermission eq 3 }
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=1">Demote to User</a></li>
			<li><a href="index.php?page=change_permission&user_id={$target_id}&value=2">Demote to Moderator</a></li>
			{/if}
		    {/if}
		    
    </td></tr>
        <tr><td height="40"></td></tr>

    </table>
<table><tr><td>
                <legend>Tickets</legend>
		<table>
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Title</th>
				    <th>Timestamp</th>
				    <th>Category</th>
				    <th>Status</th>
			    </tr>
		    </thead>   
		    <tbody>
			  {foreach from=$ticketlist item=ticket}
			  <tr>
				<td>{$ticket.tId}</td>
				<td><a href ="index.php?page=show_ticket&id={$ticket.tId}">{$ticket.title}</a></td>
				<td class="center"><i>{$ticket.timestamp}</i></td>
				<td class="center">{$ticket.category}</td>

				<td class="center"><span class="label {if $ticket.status eq 0}label-success{else if $ticket.status eq 1}label-warning{else if $ticket.status eq 2}label-important{/if}">{if $ticket.status eq 0} <i class="icon-exclamation-sign icon-white"></i>{/if} {$ticket.statusText}</span></td>  
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	    </td></tr></table>
{/block}
	

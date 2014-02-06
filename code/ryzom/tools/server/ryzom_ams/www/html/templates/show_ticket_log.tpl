{block name=content}
<div class="row-fluid sortable ui-sortable">
    <div class="box span12">
        <div class="box-header well" data-original-title="">
            <h2><i class="icon-tag"></i> Log of Ticket #{$ticket_id}</h2>
            <div class="box-icon">
                <a href="#" class="btn btn-minimize btn-round"><i class="icon-chevron-up"></i></a>
                <a href="#" class="btn btn-close btn-round"><i class="icon-remove"></i></a>
            </div>
        </div>
        <div class="box-content">
            <div class="row-fluid">
                <legend>Title: <a href="index.php?page=show_ticket&id={$ticket_id}">{$ticket_title}</a></legend>
		<table class="table table-striped table-bordered bootstrap-datatable datatable">
		    <thead>
			    <tr>
				    <th>ID</th>
				    <th>Timestamp</th>
				    <th>Query</th>
			    </tr>
		    </thead>   
		    <tbody>
			  {foreach from=$ticket_logs item=log}
			  <tr>
				<td>{$log.tLogId}</td>
				<td><span title="{$log.timestamp_elapsed}" data-rel="tooltip"  data-placement="right">{$log.timestamp}</span></td>
				<td>{$log.query}</td>
			  </tr>
			  {/foreach}
	  
		    </tbody>
	    </table>            
	    </div>
	</div>
    </div><!--/span-->
</div><!--/row-->
{/block}
	
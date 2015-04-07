<?php
/**
 * @file
 * Symplectic Migrate Admin page callback
 */
 


/**
 * Form Construct for Symplectic Migrate Module
 */
 function drush_mymodule_custom_save_node_validate($nid){
    //check that this node is of certain type and return error if not:
      drush_set_error('You can not save this node.');
}
 
/**
 * Form Construct for Symplectic Migrate Module
 */
function symplectic_migrate_admin_settings($form_state) {

  $form['symplectic_migrate_favourites'] = array(
    '#type' => 'checkbox',
	'#title' => t('Migrate Favourite Publications'),
	'#default_value' => variable_get('symplectic_migrate_favourite',array('page')),
	'#description' => t('If selected, the favourite publications of the selected PI will also be migrated in cron process.'),
  );
  
  //Getting all PI's name form Principal Investigator content type.
  $result = db_select('node', 'n')
    ->fields('n', array('title', 'nid'))
	->condition('type', 'principal_investigator')
    ->execute();

  foreach($result as $record) {
    $list[$record->nid]=$record->title;
  }
  
  $form['symplectic_migrate_pi'] = array(
    '#type' => 'checkboxes',
	'#title' => t('Migrate publications of these PI'),
	'#options' => $list,
	'#default_value' => variable_get('symplectic_migrate_pi',array('page')),
	'#description' => t('The Publications of the selected PI will be migrated in cron process.'),
  );
  $form['manual_import'] = array(
    '#type' => 'fieldset',
	'#title' => t('Manual Symplectic Migrate'),
	'#collapsible' => FALSE,
	'#collapsed' => FALSE,
  );
  $form['manual_import']['cron_information'] = array(
	'#type' => 'markup',
    '#value' => t('These settings will be used as Cron config'),
  );
  $form['manual_import']['manual_actions'] = array('#type' => 'actions');
  $form['manual_import']['manual_actions']['allpub'] = array(
	'#type' => 'submit',
	'#value' => t('Get Symplectic Publication'),
	'#weight' => 25,
	'#submit' => array('get_symplectic_publications_submit'),
	'#validate' => array('get_symplectic_publications_validate')
  );
  return system_settings_form($form);
}


function get_symplectic_publications_submit($form,&$form_state) {


   //Getting all PI's name form Principal Investigator content type.
  $result = db_select('node', 'n')
    ->fields('n', array('title', 'nid'))
	->condition('type', 'principal_investigator')
    ->execute();

  foreach($result as $record) {
    if($form_state['input']['symplectic_migrate_pi'][$record->nid]!=NULL){

	  $principal=node_load($record->nid);
	  $symplectic_id=$principal->field_symplectic_id['und'][0]['value'];
	  
	  $machine_name_page='SymplecticPages';	   
      $migration_page = Migration::getInstance($machine_name_page);
	  if(symplectic_call($symplectic_id,1,0)){

		 // $migresult = $migration->processRollback();
	      $migresult_page = $migration_page->prepareUpdate();
          $migresult_page = $migration_page->processImport();
		  //here the principal investigator has the numer of publications
		 
		  
		  $prin=node_load($record->nid);
		  
		  $numpub=$prin->field_num_publications['und'][0]['value'];
		  $numpages=$numpub/25;
		  $pages=floor($numpages);
		  if(($numpub%25)!=0) $pages=$pages+1;
		  watchdog('symplectic_migrate','Added the num pages %i from %pi .',array('%i' => $pages,'%pi' => $principal->title),WATCHDOG_INFO);
	  }

	  $machine_name='SymplecticPubs';	   
      $migration = Migration::getInstance($machine_name);
	  $migration->setdrupalPI($form_state['values']['symplectic_migrate_pi'][$record->nid]);
	   
	  
	  for($i=1;$i<=$pages;$i++){
	    //SYmplectic Call
        // Sleep used to introduce delay between symplectic calls 
        //if($i>3) sleep(15);
    
	    if(symplectic_call($symplectic_id,$i,0)){
	
     	  //$migresult = $migration->processRollback();
		  $migresult = $migration->prepareUpdate();
          $migresult = $migration->processImport();
		  watchdog('symplectic_migrate','Page %i Symplectic Publication from %pi migrated.',array('%i' => $i,'%pi' => $principal->title),WATCHDOG_INFO);
		 	  
	    }
		else{		
		  drupal_set_message(t('An error occurred in (all) symplectic_call function and processing did not complete.'), 'error');
		  watchdog('symplectic_migrate','An error occurred when updating Symplectic Publication from %pi. The processing did not complete.',array('%pi' => $principal->title),WATCHDOG_ERROR);
		  break;
		  
		}
	  }
	}
  }

  //If Favourites is selected, we have to import them

  if($form_state['values']['symplectic_migrate_favourites']!=0) {
       	
	//Getting all PI's name form Principal Investigator content type.
    $result = db_select('node', 'n')
      ->fields('n', array('title', 'nid'))
	  ->condition('type', 'principal_investigator')
      ->execute();
	  
    foreach($result as $record) {
      if($form_state['input']['symplectic_migrate_pi'][$record->nid]!=NULL){ //The PI is selected 
	  
	    //First of all we have to delete all previous favourite publications in PI node
		$drupalPI_id = $form_state['values']['symplectic_migrate_pi'][$record->nid];
		$authornode = node_load($drupalPI_id);
	    $symplectic_id=$authornode->field_symplectic_id['und'][0]['value'];
		     		
		// Remove the field value
		unset($authornode->field_favourite_mig_pub['und']);

		// Save node
		
		node_save($authornode);
		
 
	    $machine_name='SymplecticFavPubs';
	    $migration = Migration::getInstance($machine_name);
	    $migration->setdrupalPI($form_state['values']['symplectic_migrate_pi'][$record->nid]);
		if(symplectic_call($symplectic_id,0,1)){
		  //$migresult = $migration->processRollback();
		  $migresult = $migration->prepareUpdate();
		  $migresult = $migration->processImport();
		  watchdog('symplectic_migrate','Symplectic Favourite Publication from %pi updated',array('%pi' => $authornode->title),WATCHDOG_INFO);
		}
		else{
		  $finish=1;
		  drupal_set_message(t('An error occurred in (fav) symplectic_call function and processing did not complete.'), 'error');
		  
		  watchdog('symplectic_migrate','An error occurred when updating Symplectic Favourite Publication from %pi. The processing did not complete.',array('%pi' => $authornode->title),WATCHDOG_ERROR);
		}
	  }
    }
  }
}

function get_symplectic_publications_validate($form,&$form_state) {
   //Getting all PI's name form Principal Investigator content type.
  $result = db_select('node', 'n')
    ->fields('n', array('title', 'nid'))
	->condition('type', 'principal_investigator')
    ->execute();

  $counter=0;	
  foreach($result as $record) {
    if($form_state['values']['symplectic_migrate_pi'][$record->nid]!=NULL){
	 $counter=1;
	}
  }
  if($counter==0){
    form_set_error('symplectic_migrate_pi',t('You must select at least one PI'));
  }
}

function symplectic_call($userId,$page,$favourites) {
  $username='symplecticNDMSGC';
  $password='Symplectic14';
  $ret=TRUE;
  
  if($favourites == 0) {
       /* full, this is where we will get a full list of publications of the users, 25 per page (max 25 per page)
        * In here we do not make use of $perPage attribute because if the user have more than 25 publications then
        * by default there will be at most 25 entries per page and pagination will be provided.
        */
	$URL="https://oxris.ox.ac.uk:8091/elements-api/v4.9/relationships?involving=user(" . $userId . ")&types=8&detail=full&page=".$page; 
	
  } else {
       /* ref, this is where we will get a list of references of publications of the users, max 999 per page
        * In most cases the value of $page would be 1 and value of $perPage would be 999
        */
	$URL = "https://oxris.ox.ac.uk:8091/elements-api/v4.9/relationships?involving=user(" . $userId . ")&types=8&page=1&detail=ref&per-page=999";
	$fav='favourites';
  }
 dpm($URL);
  $ch = curl_init();
  curl_setopt($ch, CURLOPT_URL,$URL);
  curl_setopt($ch, CURLOPT_TIMEOUT, 30); //timeout after 30 seconds
  curl_setopt($ch, CURLOPT_RETURNTRANSFER,1);
  curl_setopt($ch, CURLOPT_HTTPAUTH, CURLAUTH_ANY);
  curl_setopt($ch, CURLOPT_USERPWD, "$username:$password");
  $status_code = curl_getinfo($ch, CURLINFO_HTTP_CODE);   //get status code
  $result=curl_exec ($ch);
 // sleep(5);
  if($result==FALSE){
     $ret=FALSE;
	 watchdog('symplectic_migrate','Symplectic call error: Symplectic doesn\'t works.',array(),WATCHDOG_ERROR);	 
  }	 
  else{
    $file_url = DRUPAL_ROOT . '/' . drupal_get_path('module', 'symplectic_migrate') . '/sources/publications.xml';
    if (file_put_contents($file_url, $result)==FALSE){
		$ret=FALSE;
		 watchdog('symplectic_migrate','Symplectic call error occurred when writting xml.',array(),WATCHDOG_ERROR);
	}	
    else {
	  $ret=TRUE;	
	  watchdog('symplectic_migrate','Symplectic call: xml written.',array(),WATCHDOG_INFO);
	}
  }
  curl_close ($ch);
  return $ret;
  
}

 
<?php 

/**
* Implements hook_drush_command().
*/
function symplectic_migrate_drush_command(){
  $items['symplectic-publications-migrate'] = array(
    'description' => "Migrate publications from Symplectic",
    'arguments' => array(
      'npi' => 'The nid of the Principal Investigator, if =0 it list PI names wiht their nids.',
	  'fav'=> 'Migrate the favourite publications',
    ),
    'aliases' => array('spm'),
    'examples' => array(
      'drush spm 1356 0' => 'Migrate the symplectic publication of PI with nid=1356, not the favourite ones',
      'drush spm 1356 1' => 'Migrate the symplectic publication of PI with nid=1356 and also the favourite ones',
      'drush spm 0 ' => 'List the PI names with their nids',	  
      'drush spm 1 0 ' => 'Migrate all PIs publications, not the favourite ones',
      'drush spm 1 1 ' => 'Migrate all PIs publications and also the favourite ones.',
    ),
  );
  return $items;
}
         
function drush_symplectic_migrate_symplectic_publications_migrate($npi, $fav){
// Load symplectic_migrate.admin.inc from the symplectic_migrate module.
  module_load_include('inc', 'symplectic_migrate', 'symplectic_migrate.admin');

  if($npi==1) import_all_pi($fav);
  elseif($npi==0){ show_pi_list();  }
  else{
    echo("\n - Started Symplectic Publications Migrate \n"); 
    $res=get_symplectic_publications($npi,$fav);
	if($res==3) drush_set_error('ERROR_PARAMETERS',dt('This PI doesn\'t have a Symplectic Id.  MIGRATION CANCELLED.'));	
    else if($res==-1) drush_set_error('ERROR_MIGRATE',dt('An error occurred in (all) symplectic_call function and processing did not complete'));
    else echo("\n - Successfully Symplectic Publications Migrate \n"); 	
  }
}

/**
 * Drush function to migrate ALL PIs
 */
function import_all_pi($fav){

  $result = db_select('node', 'n')
       ->fields('n', array('title', 'nid'))
	   ->condition('type', 'principal_investigator')
       ->execute();
	   
  foreach($result as $record) {  
    echo("\n - Started ".$record->title."'s publications migrate \n"); 
	get_symplectic_publications($record->nid,$fav);
    echo("\n - Finished ".$record->title."'s publications migrate \n"); 
  }  
}

/**
 * Drush function to list PI name with their nid 
 */ 
function show_pi_list(){
  //Getting all PI's name form Principal Investigator content type.
  $result = db_select('node', 'n')
       ->fields('n', array('title', 'nid'))
	   ->condition('type', 'principal_investigator')
       ->execute();
	   
  foreach($result as $record) {  
    echo("- ".$record->title." (".$record->nid.")\n"); 
  }  
} 
/**
 * Drush function to validate the parameters
 */
function drush_symplectic_migrate_symplectic_publications_migrate_validate($npi, $fav){

   if(($npi==0)OR($npi==1)){
	  if($npi==-1){
		if(($fav!=0)&&($fav!=1))   drush_set_error('ERROR_PARAMETERS',dt('The second argument must be = 1 if you want to migrate the favourite, or =0 if not.'));
	  }
   }
   else{
	//check if the npi is a Principal Investigator 
	$result = db_select('node', 'n')
     ->fields('n', array('type'))
 	 ->condition('nid', $npi,'=')
     ->execute();
	
	$record = $result->fetchField();

	if((!$record) OR ($record!='principal_investigator')){
      drush_set_error('ERROR_PARAMETERS',dt('This PI id is wrong.'));	  
	}
	else{
	  //the value of FAV must be 0 or 1
	  if(($fav!=0)&&($fav!=1))   drush_set_error('ERROR_PARAMETERS',dt('The second argument must be = 1 if you want to migrate the favourite, or =0 if not.'));
	}
  }
}

/**
 * Drush function to get the symplectic publications
 */
function get_symplectic_publications($pi,$fav) {

      
	  $principal=node_load($pi);
	  $symplectic_id=$principal->field_symplectic_id['und'][0]['value'];
	  
	  if($symplectic_id==NULL) return 3;
	  
	  $machine_name_page='SymplecticPages';	   
      $migration_page = Migration::getInstance($machine_name_page);
	  if(symplectic_call($symplectic_id,1,0)){

		 // $migresult = $migration->processRollback();
	      $migresult_page = $migration_page->prepareUpdate();
          $migresult_page = $migration_page->processImport();
		  //here the principal investigator has the numer of publications
		  
		  $prin=node_load($pi);
		  
		  $numpub=$prin->field_num_publications['und'][0]['value'];
		  $numpages=$numpub/25;
		  $pages=floor($numpages);
		  if(($numpub%25)!=0) $pages=$pages+1;
		  watchdog('symplectic_migrate','Added the num pages %i from %pi .',array('%i' => $pages,'%pi' => $principal->title),WATCHDOG_INFO);
	  }
 
	  $machine_name='SymplecticPubs';	   
      $migration = Migration::getInstance($machine_name);
	  $migration->setdrupalPI($pi);
	   
	  for($i=1;$i<=$pages;$i++){
	 
	    if(symplectic_call($symplectic_id,$i,0)){
	
     	  //$migresult = $migration->processRollback();
		  $migresult = $migration->prepareUpdate();
          $migresult = $migration->processImport();
		  watchdog('symplectic_migrate','Page %i Symplectic Publication from %pi migrated.',array('%i' => $i,'%pi' => $principal->title),WATCHDOG_INFO);
		 	  
	    }
		else{		
		  drupal_set_message(t('An error occurred in (all) symplectic_call function and processing did not complete.'), 'error');
		  watchdog('symplectic_migrate','An error occurred when updating Symplectic Publication from %pi. The processing did not complete.',array('%pi' => $principal->title),WATCHDOG_ERROR);
		  return -1;
		  break;
		  
		}
	  }

  //If Favourites is selected, we have to import them

  if($fav==1) {
       	
	    //First of all we have to delete all previous favourite publications in PI node
		//$drupalPI_id = $form_state['values']['symplectic_migrate_pi'][$record->nid];
		$authornode = node_load($pi);
	    $symplectic_id=$authornode->field_symplectic_id['und'][0]['value'];
		     
		// Remove the field value
		unset($authornode->field_favourite_mig_pub['und']);
		node_save($authornode);
		
 
	    $machine_name='SymplecticFavPubs';
	    $migration = Migration::getInstance($machine_name);
	    $migration->setdrupalPI($pi);
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
		  return -1;
		}
  }
}

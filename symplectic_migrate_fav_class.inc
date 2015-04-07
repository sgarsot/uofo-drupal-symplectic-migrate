<?php


//definition of the class that will manage the import of FAVOURITES publications of an author
class SymplecticFavPubs extends XMLMigration {

  private $drupalPI=0;
  private $favcounter=0;
  
  public function setdrupalPI($value) { 
    $this->drupalPI = $value; 
  }
  public function getdrupalPI() { 
    return $this->drupalPI; 
  }
  public function __construct($arguments) {
   
    parent::__construct($arguments);//MigrateGroup::getInstance('symplectic'));
    $this->description = t('XML feed of all publication of a user');
	
    $items_url = DRUPAL_ROOT . '/' . drupal_get_path('module', 'symplectic_migrate') . '/sources/publications.xml'; //local
    
	$item_xpath = '/feed/entry';
    $item_ID_xpath = 'api:relationship/api:related/api:object/@id';

	$fields = array(
      'id' => t('ID of publication'),
      'fav' => t('Is it favourite publication?'),
    );

    $this->source = new MigrateSourceXML($items_url, $item_xpath, $item_ID_xpath, $fields);

    $this->destination = new MigrateDestinationNode('publication');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'id' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
	$migrationMachineName='SymplecticPubs';
	$this->addFieldMapping('nid', 'id')
		 ->sourceMigration(array($migrationMachineName));
	$this->addFieldMapping('favourite', 'fav')->xpath('api:relationship/api:is-favourite');
   }

  public function prepare($entity, stdClass $row) {
 
	//check if it is a favourite publication
	if($entity->favourite=='true'){
	  //we have to add it as a fav pub into PI pub_fav_list
	
	  $publication_nid=$entity->nid;
	  $PIauthor=$this->getDrupalPI();
	 
	 // Add the pubid as a favourite publication
	  $authornode = node_load($PIauthor);
	  $authornode->field_favourite_mig_pub['und'][$this->favcounter]['target_id'] = $publication_nid;	
	  $this->favcounter = $this->favcounter + 1;	  

	  node_save($authornode);
	 
	}  
  }
}

			
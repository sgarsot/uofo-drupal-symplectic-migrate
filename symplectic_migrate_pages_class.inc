<?php


//definition of the class that will manage the import of FAVOURITES publications of an author
class SymplecticPages extends XMLMigration {

  private $numpages=0;
  
  public function setnumPages($value) { 
    $this->numpages = $value; 
  }
  public function getnumPages() { 
    return $this->numpages; 
  }
  public function __construct($arguments) {
  
    parent::__construct($arguments);//MigrateGroup::getInstance('symplectic'));
    $this->description = t('XML feed the num of pages of publications of a user');
	
   //the name of the file will be like  'userid-fav.xml'
   $items_url = DRUPAL_ROOT . '/' . drupal_get_path('module', 'symplectic_migrate') . '/sources/publications.xml'; //local
    
	$item_xpath = '/feed';
    $item_ID_xpath = 'api:pagination/api:page/@href';

	$fields = array(
      'numpages' => t('Num pages publications'),
      'author' => t('author id'),
	  'mid' => t('Author name'),
    );

    $this->source = new MigrateSourceXML($items_url, $item_xpath, $item_ID_xpath, $fields);

    $this->destination = new MigrateDestinationNode('principal_investigator');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'mid' => array(
          'type' => 'varchar',
          'length' => 255,
          'not null' => TRUE,
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
	$this->addFieldMapping('title2', 'mid'); 
	$this->addFieldMapping('field_num_publications', 'numpages')->xpath('api:pagination/@results-count');

	$this->addFieldMapping('nid2', 'author')->xpath('api:pagination/api:page/@href');
   }

  public function prepare($node, stdClass $row) {

    $path=$row->author[0];
	
	//necessitem agafar els symplectic id i mirar quin author es
	$result = db_select('node', 'n')
      ->fields('n', array('nid'))
	  ->condition('type', 'principal_investigator')
      ->execute();
	
    $currentpi=0;	
    foreach($result as $record) {
	  $pii=node_load($record->nid); // obtenim el node PI

	  $idsym=$pii->field_symplectic_id['und'][0]['value'];
	  if(strpos($path,$idsym)!=FALSE){
	    $pii->field_num_publications['und'][0]['value']=$row->numpages;
		node_save($pii);
  	    $currentpi=$record->nid;
		break;
	  }	
    }	
	
	$node->title = $pii->title;
	$row->mid=$currentpi;
	$node->nid=$currentpi;   

	return TRUE;
  }
}

			
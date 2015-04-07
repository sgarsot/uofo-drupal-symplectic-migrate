<?php
//definition of the class that will manage the import of ALL publications of an author
class SymplecticPubs extends XMLMigration {
  private $drupalPI=0;
  private $NextPage=0;
  
  public function setdrupalPI($value) { 
    $this->drupalPI = $value; 
  }
  public function getdrupalPI() { 
    return $this->drupalPI; 
  }
   
  public function setNextPage($value) { 
    $this->NextPage = $value; 
  }
  public function getNextPage() { 
    return $this->NextPage; 
  }

  public function __construct($arguments) {

    parent::__construct($arguments);//MigrateGroup::getInstance('symplectic'));
    $this->description = t('XML feed of all publication of a user');

	//the name of the file will be like  'userid.xml'
    $items_url = DRUPAL_ROOT . '/' . drupal_get_path('module', 'symplectic_migrate') . '/sources/publications.xml'; //local
    
	$item_xpath = '/feed/entry/api:relationship';
    $item_ID_xpath = 'api:related/api:object/@id';

	$fields = array(
      'id' => t('ID of publication'),
      'title' => t('Title of publication'),
	  'DOI' => t('Several fields'),
	  'pubmed' =>t('Pubmed ID'),
	  'journal' =>t('Journal'),
	  'volume' =>t('Volume'),
	  'authorname' =>t('Authors'),
	  'initials' =>t('Authors Initials'),
	  'pagini' =>t('Initial page'),
	  'pagefinal' =>t('Final Page'),
	  'pubdate' =>t('Publication date'),
	  'pubdate-month' =>t('Publication date - month'),
	  'pubdate-year' =>t('Publication date - year'),
	  'visible' => t('visible publication'),
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
 
    $this->addFieldMapping('field_uid_migpub', 'id');
	$this->addFieldMapping('title', 'title')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="title"]/api:text');
	$this->addFieldMapping('field_volume_pages_migpub', 'volume')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="volume"]/api:text');
	$this->addFieldMapping('page_ini', 'pagini')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="pagination"]/api:pagination/api:begin-page');
	$this->addFieldMapping('page_final', 'pagefinal')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="pagination"]/api:pagination/api:end-page');
	$this->addFieldMapping('field_publication_date_migpub', 'pubdate')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="publication-date"]/api:date/api:day')->defaultValue('01');
	$this->addFieldMapping('month', 'pubdate-month')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="publication-date"]/api:date/api:month')->defaultValue('01');
	$this->addFieldMapping('field_year_migpub', 'pubdate-year')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="publication-date"]/api:date/api:year');
	
	$this->addFieldMapping('field_authors_migpub','authorname')->xpath('api:related/api:object/api:journal/api:records/api:record[1]/api:title')->defaultValue(NULL);
	$this->addFieldMapping('field_authors_migpub3', 'Author')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="authors"]/api:people/api:person/api:last-name')->defaultValue(NULL);
	$this->addFieldMapping('field_authors_migpub2', 'initials')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="authors"]/api:people/api:person/api:initials')->defaultValue(NULL);
	$this->addFieldMapping('field_doi_migpub', 'DOI')->xpath('api:related/api:object/api:records/api:record[1]/api:native/api:field[@name="doi"]/api:text')->defaultValue(NULL);
	$this->addFieldMapping('field_pubmed_id_migpub', 'pubmed')->xpath('api:related/api:object/api:records/api:record[@source-name="pubmed"]/@id-at-source')->defaultValue(NULL);//api:native/api:field[@name="external-identifiers"]/api:identifiers/api:identifier[@scheme="pubmed"]')->defaultValue(NULL);
	$this->addFieldMapping('field_journal_migpub','journal')->xpath('api:related/api:object/api:journal/api:records/api:record[1]/api:title')->defaultValue(NULL);
	$this->addFieldMapping('visible', 'visibility')->xpath('api:is-visible')->defaultValue(NULL); 
     
   }	

  public function prepare($node, stdClass $row) {

   	$authorlist='';
	$i=0;
	while($node->field_authors_migpub3[$i]){	
	  $authorlist.=$node->field_authors_migpub3[$i].", ".$node->field_authors_migpub2[$i].'; ';
	  $i++;
	}
	$node->field_authors_migpub['und'][0]['value']=$authorlist;
	$node->field_volume_pages_migpub['und'][0]['value'].=":".$node->page_ini."-".$node->page_final;
	
	if ($node->month!=NULL) $node->month=$node->month.	'.';
    if ($node->field_publication_date_migpub['und'][0]['value']!=NULL) $node->field_publication_date_migpub['und'][0]['value'].='.';
	
	$node->field_publication_date_migpub['und'][0]['value']= $node->month.$node->field_publication_date_migpub['und'][0]['value'].$node->field_year_migpub['und'][0]['value'];

	return TRUE;
  }
 
  public function prepareRow(stdClass $current_row) {
  

        //if the publication already exists (from another PI), we need to add this new PI as author (entity reference)
		$PIauthor=$this->getDrupalPI();
		
		///************ aixo ho hauriem de fer al complete, ja que alli sabrem si es visible o no  
		$publication_symid = $current_row->id;
        // $current_row->principals = array();
	
		
		//we need then drupal nid of the publication.
		$result = db_select('field_data_field_uid_migpub','v')
              ->fields('v',array('entity_id'))
              ->condition('field_uid_migpub_value', $publication_symid,'=')
              ->execute()->fetchField();
		
		if($result!=NULL){ 
		  //$result has the drupal nid of the current publication
		  $publication=node_load($result);
		  $i=0;
		  $found=0;
		  while($publication->field_pi_authors_migpub['und'][$i]) {
			if($publication->field_pi_authors_migpub['und'][$i]['target_id'] == $PIauthor) $found=1;
			//$current_row->principals[] = $publication->field_pi_authors_migpub['und'][$i]['target_id'];
		  	$i++;
		  }
		  if($found==0){
  		    $publication->field_pi_authors_migpub['und'][$i]['target_id']=$PIauthor;
			
		  }
		
		  node_save($publication);
		}
		else{
		  //si es la primera vegada que "insertem" aquesta publicacio, no haurem trobat el seu drupal nid i no li haurem pogut insertar el pi.
		  //guardem el pi a la $row i a la function complete($entity, stdClass $row) ho utilitzarem, ja que alli ja tindrem el node creat
		  $current_row->piauthor=$PIauthor;
		}
		return TRUE;
   }
   public function complete($entity, stdClass $row){
     
       if($row->piauthor){
	     //entrarem aqui si es la primera vegada que "insertem" aquesta publicacio
	     $publication=node_load($entity->nid);
		 if($row->visibility=='true'){ //if it's hidden we don't add the PI as a author
	       $publication->field_pi_authors_migpub['und'][0]['target_id']=$row->piauthor;
	     }
		 node_save($publication);
	   }
	 
   }
}
<?php
/**
 * @file
 * Declares our migrations.
 */
/**
 * Implements hook_migrate_api()
 */
function symplectic_migrate_migrate_api() {
  $api = array(
    'api' => 2,
    'groups' => array(
      'symplectic' => array(
        'title' => t('Symplectic Publications Import'),
      ),
    ),
    'migrations' => array(
      'AllAuthorPubs' => array(
        'class_name' => 'SymplecticPubs',
        'group_name' => 'sympletic',
      ),
	  'FavAuthorPubs' => array(
        'class_name' => 'SymplecticFavPubs',
        'group_name' => 'sympletic',
      ),
	  'PagesPubs' => array(
        'class_name' => 'SymplecticPages',
        'group_name' => 'sympletic',
      ),
    ),
  );
  return $api;
}
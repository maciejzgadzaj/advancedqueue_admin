<?php

/**
 * Implements hook_views_data_alter().
 *
 * @see advancedqueue_views_data()
 */
function advancedqueue_admin_views_data_alter(&$data) {
  // Add string filter for the Result field.
  $data['advancedqueue']['result']['filter']['handler'] = 'views_handler_filter_string';

  // Use custom handler extending views_handler_field_serialized to avoid
  // 'Undefined index' PHP notices if the value does not exist in the serialized
  // Data field.
  $data['advancedqueue']['data']['field']['handler'] = 'advancedqueue_admin_handler_field_serialized';

}

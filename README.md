# Advanced Queue Admin

## Description

_Advanced Queue Admin_ module is an extension to [_Advanced Queue_](https://www.drupal.org/project/advancedqueue) module, providing alternative admin UI for viewing and managing queue items.


## Features

* Page listing all queues defined by `hook_advanced_queue_info()` implementations and number of items in them by their status
  ![Queues](https://www.drupal.org/files/project-images/queues.png "Queues")
* Pages listing all/selected items for each defined queue
  ![Queue items](https://www.drupal.org/files/project-images/queue-items.png "Queue items")
* [_Views Bulk Operations_](https://www.drupal.org/project/views_bulk_operations) support for operations on queue items:
  - process item
  - mark item as processed
  - release item
  - requeue item
  - reset attempt counter
  - modify entity values
  - delete item
* [_Devel_](https://www.drupal.org/project/devel) module support for displaying queue/queue item details
* Additional _Drush_ commands:
  - `advancedqueue-list-items` (`aqli`) - returns a list of unprocessed items in a queue
  - `advancedqueue-delete` (`aqd`) - deletes all items from a queue
  - `advancedqueue-delete-item` (`aqdi`) - deletes a specific item from a queue
* Additional aliases for existing _Drush_ commands:
  - `aqp` - for `advancedqueue-process-queue`
  - `aql` - for `advancedqueue-list`


//      inventory_cat_items.c
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Adding items to categories.

#include <mysql.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>
#include "settings.h"
#include "messages.h"
#include "inventory_cat_structs.h"
#include "inventory_cat_items.h"

/*
TODO:


*/

/* Called upon from checkout.c */
void categoryAddItems(intrackCategories *intrackCat) {
	
	GtkBuilder *builder;
    builder = gtk_builder_new();
	intrackCat->itemContainer = (ItemContainer*) g_malloc(sizeof (ItemContainer)); /* possible memory leak? */

	intrackCat->itemContainer->data_query = g_strdup(NULL);
	
    // Load UI from file.
	gtk_builder_add_from_file(builder, CATEGORY_ADD_ITEM_FILE, NULL);
	
	intrackCat->itemContainer->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "addItemWindow"));
	g_signal_connect(intrackCat->itemContainer->mainWindow, "destroy", G_CALLBACK(freeMemory), intrackCat);
	
	intrackCat->itemContainer->selectedItemCode = g_strconcat(NULL, NULL);
	
	// Setup the widget window
    gtk_window_set_title (GTK_WINDOW(intrackCat->itemContainer->mainWindow), "Add Item");
	gtk_window_set_modal(GTK_WINDOW(intrackCat->itemContainer->mainWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(intrackCat->itemContainer->mainWindow), TRUE);
	//gtk_widget_set_size_request(intrackCat->itemContainer->mainWindow, 324, 432);
	gtk_window_set_deletable(GTK_WINDOW(intrackCat->itemContainer->mainWindow), FALSE);
	gtk_window_set_position(GTK_WINDOW(intrackCat->itemContainer->mainWindow), GTK_WIN_POS_CENTER);
	gtk_container_set_border_width(GTK_CONTAINER(intrackCat->itemContainer->mainWindow), 0);
	gtk_window_set_transient_for(GTK_WINDOW(intrackCat->itemContainer->mainWindow), GTK_WINDOW(intrackCat->mainWindow));
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(intrackCat->itemContainer->mainWindow), TRUE);
	
	
	// Setup the buttons
	intrackCat->itemContainer->cancelButton = GTK_WIDGET(gtk_builder_get_object(builder, "cancelButton"));
	g_signal_connect(intrackCat->itemContainer->cancelButton, "clicked", G_CALLBACK(destroyWindow), intrackCat);

	intrackCat->itemContainer->okButton = GTK_WIDGET(gtk_builder_get_object(builder, "okButton"));
	g_signal_connect(intrackCat->itemContainer->okButton, "clicked", G_CALLBACK(prepareItemAdd), intrackCat);
	
	intrackCat->itemContainer->inventorySearchButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventorySearchButton"));
	g_signal_connect(intrackCat->itemContainer->inventorySearchButton, "clicked", G_CALLBACK(getInventory), intrackCat);

	intrackCat->itemContainer->inventorySearchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "inventorySearchEntry"));
	g_signal_connect(intrackCat->itemContainer->inventorySearchEntry, "activate", G_CALLBACK(getInventory), intrackCat);
	
	intrackCat->itemContainer->barcodeSearch = GTK_WIDGET(gtk_builder_get_object(builder, "barcodeSearch"));
	intrackCat->itemContainer->nameSearch = GTK_WIDGET(gtk_builder_get_object(builder, "nameSearch"));
	intrackCat->itemContainer->descriptionSearch = GTK_WIDGET(gtk_builder_get_object(builder, "descriptionSearch"));
	intrackCat->itemContainer->manufacturerSearch = GTK_WIDGET(gtk_builder_get_object(builder, "manufacturerSearch"));
	intrackCat->itemContainer->categorySearch = GTK_WIDGET(gtk_builder_get_object(builder, "categorySearch"));
	
	intrackCat->itemContainer->searchSpinMin = GTK_WIDGET(gtk_builder_get_object(builder, "searchSpinMin"));
	intrackCat->itemContainer->searchSpinMax = GTK_WIDGET(gtk_builder_get_object(builder, "searchSpinMax"));	
	intrackCat->itemContainer->costSearch = GTK_WIDGET(gtk_builder_get_object(builder, "costSearch"));
	intrackCat->itemContainer->priceSearch = GTK_WIDGET(gtk_builder_get_object(builder, "priceSearch"));
	intrackCat->itemContainer->stockSearch = GTK_WIDGET(gtk_builder_get_object(builder, "stockSearch"));
	intrackCat->itemContainer->inventoryNumberSearchButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryNumberSearchButton"));
	g_signal_connect(intrackCat->itemContainer->inventoryNumberSearchButton, "clicked", G_CALLBACK(getInventoryNumbers), intrackCat);

	// Setup popup menu when right clicking on the inventory tree
	intrackCat->itemContainer->invenMenu = gtk_menu_new();
	
	intrackCat->itemContainer->invenMenuView = gtk_menu_item_new_with_label("View item");
	gtk_menu_shell_append(GTK_MENU_SHELL(intrackCat->itemContainer->invenMenu), intrackCat->itemContainer->invenMenuView);
	gtk_widget_show(intrackCat->itemContainer->invenMenuView);
	g_signal_connect(G_OBJECT(intrackCat->itemContainer->invenMenuView), "activate", G_CALLBACK(prepareViewWindow), intrackCat);
	gtk_widget_set_sensitive(intrackCat->itemContainer->invenMenuView, FALSE);
	
	// Setup the tree
	intrackCat->itemContainer->inventoryContainer = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryViewport"));
	intrackCat->itemContainer->inventoryTree = gtk_tree_view_new();
	setupTree(intrackCat->itemContainer);
	gtk_container_add(GTK_CONTAINER(intrackCat->itemContainer->inventoryContainer), intrackCat->itemContainer->inventoryTree);
	
	GtkTreeSelection *selection2;
	selection2 = gtk_tree_view_get_selection(GTK_TREE_VIEW (intrackCat->itemContainer->inventoryTree));
	gtk_tree_selection_set_mode(selection2, GTK_SELECTION_MULTIPLE);	
	
	getInventory(NULL, intrackCat); // Loadup the inventory list
	
	// button clicks and keypress signals on the tree
	g_signal_connect(intrackCat->itemContainer->inventoryTree, "button-press-event", G_CALLBACK(treeButtonPress), intrackCat);
	g_signal_connect(intrackCat->itemContainer->inventoryTree, "key-press-event", G_CALLBACK(treeKeyPress), intrackCat);	
	
	gtk_widget_show_all(intrackCat->itemContainer->mainWindow);
    g_object_unref(G_OBJECT(builder));
	
}

/*
start -> destroyWindow()
start -> freeMemory() : widget*
finished -> freeMemory() : widget*

29 seconds -> the treeview/store is taking long time to be destroyed.

finished -> destroyWindow()
*/
static void destroyWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	//gtk_widget_hide_all(intrackCat->itemContainer->mainWindow);
	clear_tree_store(intrackCat);
	gtk_widget_destroy(GTK_WIDGET(intrackCat->itemContainer->mainWindow));
}

static void freeMemory(GtkWidget *widget, intrackCategories *intrackCat) {
	g_free(intrackCat->itemContainer->selectedItemCode);
	g_free(intrackCat->itemContainer->data_query);
	g_free(intrackCat->itemContainer);
	//gtk_widget_destroy(widget);
	
	//gtk_widget_hide_all(intrackCat->itemContainer->mainWindow);
	
	/*
	if(widget) {
		//destroyWindow(NULL, intrackCat->itemContainer);
		//gtk_widget_destroy(widget);
	}
	*/
}

// Clears the tree store, quicker to erase from memory when the tree is empty.
static void clear_tree_store(intrackCategories *intrackCat) {
	GtkListStore *store;
	GtkTreeIter 	iter;

	store = gtk_list_store_new (INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);

	gtk_list_store_append (store, &iter);
			
	gtk_list_store_set (store, &iter, ID, "", BARCODE, "", DESCRIPTION, "", MANUFACTURER, "", CATEGORY, "", COST, "", COSTAVG, "", PRICE, "", STOCK, "", -1);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);		
}

// Add a item from the inventory database to the category inventory
static void beginItemAdd(GtkTreeRowReference *ref, intrackCategories *intrackCat) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData;
	gchar *tempData;
	
	if(intrackCat->selectedCat != NULL) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree));
		path = gtk_tree_row_reference_get_path (ref);
		gtk_tree_model_get_iter (model, &iter, path);

		gtk_tree_model_get(model, &iter, ID, &whereData, -1);

		tempData = g_strdup(intrackCat->itemContainer->data_query);
		g_free(intrackCat->itemContainer->data_query);
		
		if(intrackCat->itemContainer->counter == 0)
			intrackCat->itemContainer->data_query = g_strdup(whereData);
		else
			intrackCat->itemContainer->data_query = g_strconcat(tempData, ", ", whereData, NULL);
		// Update the database
		//inventoryModifyCategory(intrackCat->selectedCat, whereData, "id"); // -> inventory_cat.c
		
		// Remove the row from the tree
		//gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		g_free(tempData);
		g_free(whereData);
		intrackCat->itemContainer->counter++;
	}
}

// Prepares to add items to the category inventory
static int prepareItemAdd(GtkWidget *widget, intrackCategories *intrackCat) {
	
	if(intrackCat->itemContainer->selectedItemCode == NULL) {
		printMessage("Select a item to add to the category", intrackCat->itemContainer->mainWindow);
		return 1;
	}

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	// Create tree row references to all of the selected rows
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	g_free(intrackCat->itemContainer->data_query);
	intrackCat->itemContainer->data_query = g_strdup("");
	intrackCat->itemContainer->counter = 0;
	
	if(references != NULL) {
		// Add each of the selected rows pointed to by the row reference
		g_list_foreach(references, (GFunc) beginItemAdd, intrackCat);
	} else {
		printMessage("ERROR: Select items to add.", intrackCat->itemContainer->mainWindow);
	}	
	
	gchar *temp_data;
	temp_data = g_strdup(intrackCat->itemContainer->data_query);
	
	g_free(intrackCat->itemContainer->data_query);
	
	intrackCat->itemContainer->data_query = g_strconcat("UPDATE `", INVENTORY_TABLES, "` SET category='", intrackCat->selectedCat, "' WHERE id IN (", temp_data, ")", NULL);
	
	g_free(temp_data);
	//g_print("%s\n", intrackCat->itemContainer->data_query);
	database_query(MILLER_PARTS_DATABASE, intrackCat->itemContainer->data_query);
	//query_string = g_strconcat("UPDATE `", INVENTORY_TABLES, "` SET category='", selectedGroup, "' WHERE id IN (", intrackCat->data, ")", NULL);
	//id IN (5, 7, 10, 11, 16, 25);
	
	// Refresh the inventory group tree
	//getGroupInventory(NULL, inventory);
	getCatInventory(intrackCat->catInventoryTree, intrackCat->selectedCat);

	// Free the tree paths, tree row references and lists
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	// Close the window
	//freeMemory(intrackCat->itemContainer->mainWindow, intrackCat);
	destroyWindow(intrackCat->itemContainer->mainWindow, intrackCat);
	
	return 0;
}

// Open up a window which shows the item information (picture, info, stats, etc)
static void prepareViewWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	loadViewItem(intrackCat->itemContainer->mainWindow, intrackCat->itemContainer->selectedItemCode);
}

// Setup and create the inventory tree
static void setupTree(ItemContainer *addItem) {
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	/*
	GtkListStore 	*store;
	GtkTreeIter 	iter;
		
	store = gtk_list_store_new (INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	*/
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);		
		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Barcode", renderer, "text", BARCODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BARCODE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", DESCRIPTION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DESCRIPTION); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Manufacturer", renderer, "text", MANUFACTURER, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, MANUFACTURER); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", CATEGORY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CATEGORY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Cost", renderer, "text", COST, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COST); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Avg Cost", renderer, "text", COSTAVG, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COSTAVG); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", PRICE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PRICE);
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Stock", renderer, "text", STOCK, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, STOCK);
	gtk_tree_view_append_column (GTK_TREE_VIEW (addItem->inventoryTree), column);		
	
	/*
	gtk_tree_view_set_model(GTK_TREE_VIEW(addItem->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);
	*/
}

// NOT USED DELETE
static int checkIfExist(gchar *barcode) {
	
	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;
	query_string = g_strconcat("SELECT barcode FROM ", mysqlTables, " WHERE barcode = '", barcode, "'", NULL);
	
	int query_state;
	query_state = mysql_query(connection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
	}
	g_free(query_string);

	//If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	
	// If the item is not in the database, then stop and return back.
	if (!mysql_fetch_row(result)) {
		mysql_free_result(result); // Free up some memory.
		mysql_close(connection);
	
		gchar *errorMessage = g_strconcat(barcode, " Item Not Found", NULL);

		//printMessage(errorMessage);
		
		g_free(errorMessage);

		return 1;
	}
	
	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);	

	return 0;
}

/* Pulls inventory data when searching by numbers ie: cost, price and stock */
static void getInventoryNumbers(GtkWidget *widget, intrackCategories *intrackCat) {
	/* Clear out selected item before refreshing the tree */
	g_free(intrackCat->itemContainer->selectedItemCode);
	intrackCat->itemContainer->selectedItemCode = g_strconcat(NULL, NULL);
		
	GtkListStore *store;
	
	store = gtk_list_store_new (INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	float min, max;
	
	min = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackCat->itemContainer->searchSpinMin));
	max = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackCat->itemContainer->searchSpinMax));
	
	if(max >= min) {
		gchar *minChar, *maxChar;
		minChar = g_strdup_printf("%f", min);
		maxChar = g_strdup_printf("%f", max);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->costSearch)))
			pullInventory(store, "cost", NULL, minChar, maxChar);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->priceSearch)))
			pullInventory(store, "price", NULL, minChar, maxChar);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->stockSearch)))
			pullInventory(store, "stock", NULL, minChar, maxChar);
			
		g_free(minChar);
		g_free(maxChar);
	}
	else
		printMessage(ERROR_SEARCH_NUMBER, intrackCat->itemContainer->mainWindow);

	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);	
	
}

// Prepare to pull inventory data from the database
static void getInventory(GtkWidget *widget, intrackCategories *intrackCat) {
	/* Clear out selected item before refreshing the tree */
	g_free(intrackCat->itemContainer->selectedItemCode);
	intrackCat->itemContainer->selectedItemCode = g_strconcat(NULL, NULL);
	
	GtkListStore *store;
	
	store = gtk_list_store_new (INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	/* Checks to see if a ' is entered, that causes mysql.h to break */
	if(g_strrstr(gtk_entry_get_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry)), "'")) {
		printMessage("ERROR: ' not allowed.", intrackCat->itemContainer->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry), ""); /* Clear out the search entry */
	}
	/* This will search the inventory for the user query if the search entry is greater than 2 characters */
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry))) > 2) {
		gchar *searchString;
		gchar *searchTemp;
		gchar *searchFinal;
		int counter = 0;
				
		searchString = g_strconcat(gtk_entry_get_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry)), NULL);
		
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->barcodeSearch))) {
			searchFinal = g_strconcat("partNo LIKE '%", searchString, "%'", NULL);

			counter++;
			//pullInventory(store, "partNo", searchString, NULL, NULL);
		}
		
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->descriptionSearch))) {
			if(counter > 0) {
				searchTemp = g_strconcat(searchFinal, " OR description LIKE '%", searchString, "%'", NULL);
				g_free(searchFinal);
				searchFinal = g_strdup(searchTemp);
				g_free(searchTemp);
			}
			else
				searchFinal = g_strconcat("description LIKE '%", searchString, "%'", NULL);	
				
			counter++;
			//pullInventory(store, "description", searchString, NULL, NULL);
		}
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->manufacturerSearch))) {
			if(counter > 0) {
				searchTemp = g_strconcat(searchFinal, " OR manufacturer LIKE '%", searchString, "%'", NULL);
				g_free(searchFinal);
				searchFinal = g_strdup(searchTemp);
				g_free(searchTemp);
			}
			else
				searchFinal = g_strconcat("manufacturer LIKE '%", searchString, "%'", NULL);
						
			counter++;
			//pullInventory(store, "manufacturer", searchString, NULL, NULL);
		}
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->itemContainer->categorySearch))) {
			if(counter > 0) {
				searchTemp = g_strconcat(searchFinal, " OR display_name LIKE '%", searchString, "%'", NULL);
				g_free(searchFinal);
				searchFinal = g_strdup(searchTemp);
				g_free(searchTemp);
			}
			else
				searchFinal = g_strconcat("display_name LIKE '%", searchString, "%'", NULL);
						
			counter++;
			//pullInventory(store, "category", searchString, NULL, NULL);
		}
		
		pullInventory(store, searchFinal, searchString, NULL, NULL);
		
		g_free(searchFinal);
		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry), "");
	}
	/* If the search entry is less than 3 characters but greater than 0 */
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, intrackCat->itemContainer->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(intrackCat->itemContainer->inventorySearchEntry), "");
	}
	else
		pullInventory(store, NULL, NULL, NULL, NULL); /* This pulls all the inventory out of the database */
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);
}

/* Pulls the inventory data out of the database and stores it into a GtkListStore */
static int pullInventory(GtkListStore *store, gchar *searchWhere, gchar *searchString, gchar *searchNumMin, gchar *searchNumMax) {
	GtkTreeIter 	iter;
	
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;		
	
	/* Open MYSQL connection to the database */
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    query_string = g_strconcat(mysqlDatabase, NULL);	
	query_state = mysql_select_db(connection, query_string);	
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}	
	
	g_free(query_string);

	/* Search for the user entered query */
	if(searchString != NULL)
		query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.manufacturer, b.display_name, a.cost, a.costAvg, a.price, a.stock FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id", "  WHERE ", searchWhere, NULL);
	else if(searchNumMin != NULL && searchNumMax != NULL)
		query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.manufacturer, b.display_name, a.cost, a.costAvg, a.price, a.stock FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id", " WHERE ", searchWhere, " >= ", searchNumMin, " AND ", searchWhere, " <= ", searchNumMax, NULL);
	else	
		query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.manufacturer, b.display_name, a.cost, a.costAvg, a.price, a.stock FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id", NULL);

	mysql_query(connection, query_string);

	//If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
	g_free(query_string);	

	while ((row = mysql_fetch_row(result))) {
		gchar *id, *barcode, *description, *manufacturer, *category, *cost, *costAverage, *price, *stock;
		
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				id = g_strdup(row[i]);
			
			if(i == 1)
				barcode = g_strdup(row[i]);
			
			if(i == 2)
				description = g_strdup(row[i]);
				
			if(i == 3)
				manufacturer = g_strdup(row[i]);

			if(i == 4)
				category = g_strdup(row[i]);
				
			if(i == 5){
				cost = g_strdup_printf("%.2f", atof(row[i]));
				//cost = g_strconcat(partsRow[i], NULL);
			}

			if(i == 6){
				costAverage = g_strdup_printf("%.2f", atof(row[i]));
				//costAverage = g_strdup_printf("%.2f", getAverageCost(barcode)); //costAverage = g_strconcat(row[i], NULL);
			}

			if(i == 7)
				price = g_strdup(row[i]);
				
			if(i == 8)
				stock = g_strdup(row[i]);
		}
		
		/* Now search the tree store to see if the item already exists in it, we search via the ID for identification. This prevents duplicates from getting displayed */
      	if(searchString != NULL || (searchNumMin != NULL && searchNumMax != NULL)) {
			gchar *rowtext;
			gboolean foundItem = FALSE;
			
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
				gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, 0, &rowtext, -1); // the ID column
		
				if(rowtext != NULL) {
					// Found item
					if(!strcmp(rowtext, id))
						foundItem = TRUE;
						
					g_free(rowtext);
				}
				
				// Finish off searching the rest of the rows in the store
				while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)) {			
					gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, 0, &rowtext, -1); // the ID column

					if(rowtext != NULL) {
						// Found item
						if(!strcmp(rowtext, id)) {
							foundItem = TRUE;
							g_free(rowtext);
							break;
						}
						g_free(rowtext);
					}
				}		
			}
			
			// If we didn't find the item in the store, then add it
			if(foundItem == FALSE) {
				gtk_list_store_append (store, &iter);
					
				gtk_list_store_set (store, &iter, ID, id, BARCODE, barcode, DESCRIPTION, description, MANUFACTURER, manufacturer, CATEGORY, category, COST, cost, COSTAVG, costAverage, PRICE, price, STOCK, stock, -1);	
			}			
		}
		else {
			/* This stores all inventory into the tree */
			gtk_list_store_append (store, &iter);
			
			gtk_list_store_set (store, &iter, ID, id, BARCODE, barcode, DESCRIPTION, description, MANUFACTURER, manufacturer, CATEGORY, category, COST, cost, COSTAVG, costAverage, PRICE, price, STOCK, stock, -1);
		}
		
		g_free(id);
		g_free(barcode);
		g_free(description);
		g_free(manufacturer);
		g_free(category);
		g_free(cost);
		g_free(costAverage);
		g_free(price);
		g_free(stock);
	}	
	
	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}

/* Gets the average cost of the item from the database */
//NOT USED DELETE
static float getAverageCost(gchar *barcode) {
	
	gchar *costQuery;
	int query_state, num_fields, i;
	float costAverage;
	
	MYSQL *costConnection, costMysql;
	MYSQL_RES *costResult;
	MYSQL_ROW costRow;
	
	mysql_init(&costMysql);
	costConnection = mysql_real_connect(&costMysql, mysqlServer, mysqlUsername, mysqlPassword, MYSQL_DEFAULT_DATABASE, 0, 0, 0);
	
    costQuery = g_strconcat(mysqlDatabase, NULL);	
	query_state = mysql_select_db(costConnection, costQuery);		
	
	g_free(costQuery);
	costQuery = g_strconcat("SELECT costAverage FROM `", mysqlTables, "` WHERE barcode = '", barcode, "'", NULL);
	query_state = mysql_query(costConnection, costQuery);

	costResult = mysql_store_result(costConnection);
	num_fields = mysql_num_fields(costResult);
			
	while ( ( costRow = mysql_fetch_row(costResult)) ) {
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				costAverage = atof(costRow[i]);
		}
	}

	g_free(costQuery);
	
	mysql_free_result(costResult);
	mysql_close(costConnection);

	return costAverage;
}

/* Mouse click event on the tree rules */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackCategories *intrackCat) {

    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
		gtk_widget_set_sensitive(intrackCat->itemContainer->invenMenuView, FALSE);
        return FALSE;
	}

    /* LMB */
    switch(ev->button)
    {
        case 1: /* 1 = left click */
            break;

        case 3: /* 3 = right click */
				gtk_widget_set_sensitive(intrackCat->itemContainer->invenMenuView, TRUE);
				gtk_menu_popup(GTK_MENU(intrackCat->itemContainer->invenMenu), NULL, NULL, NULL, NULL, 3, ev->time);
            break;
    }
    
	/* Keep track of the selected item */
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gchar *rowBarcode;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, BARCODE, &rowBarcode, -1);
    
    g_free(intrackCat->itemContainer->selectedItemCode);
    intrackCat->itemContainer->selectedItemCode = g_strconcat(rowBarcode, NULL);
    g_free(rowBarcode);
    
    /* free our path */
    gtk_tree_path_free(path);  

    return FALSE;	
}

/* Keyboard key press event on the inventory tree */
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, intrackCategories *intrackCat) {
	
    switch(ev->keyval)
    {       
        case GDK_Up:
				/* Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection. */
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;

        case GDK_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;

        case GDK_KP_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;  
            
        case GDK_End:
				g_timeout_add (1, keyPressSelection, (gpointer) intrackCat); 		
            break;
    }
	
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {
    /*
    intrackCategories *intrackCat = (intrackCategories *)data;
    
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean pathSelected;
    
    gchar *rowtext;
    
    // Need to pull the current selected row from the treeview 
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->itemContainer->inventoryTree));

	// Now need to pull the path and iter from the selected row 
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		pathSelected = TRUE;
	}
	else {
		pathSelected = FALSE;
		
		return TRUE;
	}	
	
	if(pathSelected) {

		if(!gtk_tree_model_get_iter(model, &iter, path)) {
			// If the iter is invalid, it means the path is invalid, so go back to the prev path and grab the iter again
			gtk_tree_path_prev(path);
			gtk_tree_model_get_iter(model, &iter, path);
		}
			
		gtk_tree_model_get(model, &iter, 1, &rowtext, -1); // barcode getting pulled
		
		g_free(intrackCat->itemContainer->selectedItemCode);
		intrackCat->itemContainer->selectedItemCode = g_strconcat(rowtext, NULL);
		
		g_free(rowtext);
	}
	
	if(pathSelected) {
		// free our path
		gtk_tree_path_free(path);
	}
	*/	
	return FALSE;
}

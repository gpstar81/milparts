//      inventory_cat.c
//      Copyright 2010-2014 Michael Rajotte <michael@michaelrajotte.com>
// 		Creating item categories and adding items to them.

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
#include "inventory_cat.h"

/*
TODO:
*/


void initalizeCategories(GtkWidget *mainWindow) {
	
	GtkBuilder *builder;
	builder = gtk_builder_new();	
	gtk_builder_add_from_file(builder, INVENTORY_CATEGORY_FILE, NULL);
	
	intrackCategories *intrackCat;
	intrackCat = (intrackCategories*) g_malloc (sizeof (intrackCategories));
	
	intrackCat->mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "categoriesWindow"));
	intrackCat->selectedCat = NULL;
	intrackCat->selectedItemCode = NULL;
	
	gtk_window_set_transient_for(GTK_WINDOW(intrackCat->mainWindow), GTK_WINDOW(mainWindow));
	gtk_window_set_modal(GTK_WINDOW(intrackCat->mainWindow), TRUE);
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(intrackCat->mainWindow), TRUE);
	
    // Close the window
    GtkWidget	*catCloseButton;
    catCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "catCloseButton"));
   	g_signal_connect(G_OBJECT(catCloseButton), "clicked", G_CALLBACK(closeWindow), intrackCat);			
	
	// Setup the category tree
	intrackCat->catViewport = GTK_WIDGET(gtk_builder_get_object(builder, "categoryScroll"));
	intrackCat->catTree = gtk_tree_view_new();
	setupCategoryTree(intrackCat);
	gtk_container_add(GTK_CONTAINER(intrackCat->catViewport), intrackCat->catTree);
	
	//GtkTreeSelection *selection3;
	//selection3 = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->catTree));
	//gtk_tree_selection_set_mode(selection3, GTK_SELECTION_MULTIPLE);	
	
	// Load the category tree
	getCategories(NULL, intrackCat);
	
	// The signals for the cat tree
	g_signal_connect(intrackCat->catTree, "button-press-event", G_CALLBACK(cat_button_press), intrackCat); 				
	g_signal_connect(intrackCat->catTree, "key-press-event", G_CALLBACK(cat_key_press), intrackCat); 	
	
	// Setup the category inventory tree
	intrackCat->catInventoryViewport = GTK_WIDGET(gtk_builder_get_object(builder, "categoryScroll2"));
	intrackCat->catInventoryTree = gtk_tree_view_new();
	setupCatInventoryTree(intrackCat);
	gtk_container_add(GTK_CONTAINER(intrackCat->catInventoryViewport), intrackCat->catInventoryTree);
	g_signal_connect(intrackCat->catInventoryTree, "button-press-event", G_CALLBACK(treeButtonPress), intrackCat);
	
	GtkTreeSelection *selection;
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->catInventoryTree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	
	// Setup popup menu when right clicking on the inventory tree
	intrackCat->invenMenu = gtk_menu_new();
	
	intrackCat->invenMenuView = gtk_menu_item_new_with_label("View item");
	gtk_menu_shell_append(GTK_MENU_SHELL(intrackCat->invenMenu), intrackCat->invenMenuView);
	gtk_widget_show(intrackCat->invenMenuView);
	g_signal_connect(G_OBJECT(intrackCat->invenMenuView), "activate", G_CALLBACK(prepareViewWindow), intrackCat);
	gtk_widget_set_sensitive(intrackCat->invenMenuView, FALSE);	
	
	// Setup the inventory tree
	/*
	intrackCat->inventoryViewport = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryViewport1"));
	intrackCat->inventoryTree = gtk_tree_view_new();
	setupInventoryTree(intrackCat);
	gtk_container_add(GTK_CONTAINER(intrackCat->inventoryViewport), intrackCat->inventoryTree);
	
	GtkTreeSelection *selection2;
	selection2 = gtk_tree_view_get_selection(GTK_TREE_VIEW (intrackCat->inventoryTree));
	gtk_tree_selection_set_mode(selection2, GTK_SELECTION_MULTIPLE);
	*/
	
	/* Setup the inventory search checkbox buttons */
	/*
	intrackCat->barcodeSearch = GTK_WIDGET(gtk_builder_get_object(builder, "barcodeSearch1"));
	intrackCat->nameSearch = GTK_WIDGET(gtk_builder_get_object(builder, "nameSearch1"));
	intrackCat->descriptionSearch = GTK_WIDGET(gtk_builder_get_object(builder, "descriptionSearch1"));
	intrackCat->manufacturerSearch = GTK_WIDGET(gtk_builder_get_object(builder, "manufacturerSearch1"));
	intrackCat->categorySearch = GTK_WIDGET(gtk_builder_get_object(builder, "categorySearch1"));
	intrackCat->costSearch = GTK_WIDGET(gtk_builder_get_object(builder, "costSearch1")); 
	intrackCat->priceSearch = GTK_WIDGET(gtk_builder_get_object(builder, "priceSearch1")); 
	intrackCat->stockSearch = GTK_WIDGET(gtk_builder_get_object(builder, "stockSearch1"));
	
	intrackCat->inventorySearchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "inventorySearchEntry1"));
	g_signal_connect(G_OBJECT(intrackCat->inventorySearchEntry), "activate", G_CALLBACK(getInventory), intrackCat);

	GtkWidget	*inventorySearchButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventorySearchButton1"));
	g_signal_connect(G_OBJECT(inventorySearchButton), "clicked", G_CALLBACK(getInventory), intrackCat);
	
	intrackCat->searchSpinMin = GTK_WIDGET(gtk_builder_get_object(builder, "searchSpinMin1"));
	intrackCat->searchSpinMax = GTK_WIDGET(gtk_builder_get_object(builder, "searchSpinMax1"));
	g_signal_connect(G_OBJECT(intrackCat->searchSpinMin), "activate", G_CALLBACK(getInventoryNumbers), intrackCat);
	g_signal_connect(G_OBJECT(intrackCat->searchSpinMax), "activate", G_CALLBACK(getInventoryNumbers), intrackCat);

	GtkWidget	*inventoryNumberSearchButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryNumberSearchButton1"));
	g_signal_connect(G_OBJECT(inventoryNumberSearchButton), "clicked", G_CALLBACK(getInventoryNumbers), intrackCat);	
	*/
	
	/* Setup the buttons */
	//intrackCat->addCatButton = GTK_WIDGET(gtk_builder_get_object(builder, "addCatButton"));
	intrackCat->addCatButtonBar = GTK_WIDGET(gtk_builder_get_object(builder, "addCatButtonBar"));
	//intrackCat->deleteCatButton = GTK_WIDGET(gtk_builder_get_object(builder, "deleteCatButton"));
	intrackCat->deleteCatButtonBar = GTK_WIDGET(gtk_builder_get_object(builder, "deleteCatButtonBar"));	
	//g_signal_connect(G_OBJECT(intrackCat->addCatButton), "clicked", G_CALLBACK(createCatWindow), intrackCat);
	g_signal_connect(G_OBJECT(intrackCat->addCatButtonBar), "clicked", G_CALLBACK(createCatWindow), intrackCat);
	//g_signal_connect(G_OBJECT(intrackCat->deleteCatButton), "clicked", G_CALLBACK(deleteCatWindow), intrackCat);	
	g_signal_connect(G_OBJECT(intrackCat->deleteCatButtonBar), "clicked", G_CALLBACK(deleteCatWindow), intrackCat);	
	
	//intrackCat->addCatItems = GTK_WIDGET(gtk_builder_get_object(builder, "addCatItems"));
	intrackCat->addCatItemsBar = GTK_WIDGET(gtk_builder_get_object(builder, "addCatItemsBar"));
	//intrackCat->deleteCatItems = GTK_WIDGET(gtk_builder_get_object(builder, "deleteCatItems"));
	intrackCat->deleteCatItemsBar = GTK_WIDGET(gtk_builder_get_object(builder, "deleteCatItemsBar"));
	//g_signal_connect(G_OBJECT(intrackCat->addCatItems), "clicked", G_CALLBACK(prepareLoadItemAdd), intrackCat);
	g_signal_connect(G_OBJECT(intrackCat->addCatItemsBar), "clicked", G_CALLBACK(prepareLoadItemAdd), intrackCat);
	//g_signal_connect(G_OBJECT(intrackCat->deleteCatItems), "clicked", G_CALLBACK(prepareItemRemove), intrackCat);
	//g_signal_connect(G_OBJECT(intrackCat->deleteCatItemsBar), "clicked", G_CALLBACK(prepareItemRemove), intrackCat);
	g_signal_connect(G_OBJECT(intrackCat->deleteCatItemsBar), "clicked", G_CALLBACK(removeItemsWindow), intrackCat);
	
	//gtk_widget_set_sensitive(intrackCat->deleteCatButton, FALSE);
	gtk_widget_set_sensitive(intrackCat->deleteCatButtonBar, FALSE);
	//gtk_widget_set_sensitive(intrackCat->addCatItems, FALSE);
	gtk_widget_set_sensitive(intrackCat->addCatItemsBar, FALSE);
	//gtk_widget_set_sensitive(intrackCat->deleteCatItems, FALSE);
	gtk_widget_set_sensitive(intrackCat->deleteCatItemsBar, FALSE);
	
	g_signal_connect(intrackCat->mainWindow, "destroy", G_CALLBACK(catFreeMemory), intrackCat); /* Free memory when the window widget is destroyed */
	
    // Connect any signals in the builder file.
    gtk_builder_connect_signals(builder, NULL);
 
	// Unref and free the memory builder used when loading.
    g_object_unref(G_OBJECT(builder));
    	
	gtk_widget_show_all(intrackCat->mainWindow);
}

static void closeWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	clear_tree_store(intrackCat);
	gtk_widget_destroy(intrackCat->mainWindow);
}

/* Free up all the memory used by the category window and structure */
static void catFreeMemory(GtkWidget *widget, intrackCategories *intrackCat) {
	g_free(intrackCat->selectedCat);
	g_free(intrackCat->selectedItemCode);
	
	clear_tree_store(intrackCat);
	
	if(widget)
		gtk_widget_destroy(widget);
		
	g_free(intrackCat);
}

/* Open up a window which shows the item information (picture, info, stats, etc) */
static void prepareViewWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	loadViewItem(intrackCat->mainWindow, intrackCat->selectedItemCode);
}

static void prepareLoadItemAdd(GtkWidget *widget, intrackCategories *intrackCat) {
	if(intrackCat->selectedCat == NULL) {
		printMessage("ERROR: Select a category.", intrackCat->mainWindow);
	}
	else
		categoryAddItems(intrackCat); // -> inventory_cat_items.c
}

/* Remove a item from the category inventory */
static void beginItemRemove(GtkTreeRowReference *ref, intrackCategories *intrackCat) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData;

	if(intrackCat->selectedCat != NULL) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catInventoryTree));
		path = gtk_tree_row_reference_get_path (ref);
		gtk_tree_model_get_iter (model, &iter, path);

		gtk_tree_model_get(model, &iter, CAT_ID, &whereData, -1);

		/* Update the database */
		inventoryModifyCategory("", whereData, "id");

		/* Remove the row from the tree */
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		g_free(whereData);
	}
}

// Remove a item from the category inventory
static void beginCatRemove(GtkTreeRowReference *ref, intrackCategories *intrackCat) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData;
	
	if(intrackCat->selectedCat != NULL) {
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catTree));
		path = gtk_tree_row_reference_get_path (ref);
		gtk_tree_model_get_iter (model, &iter, path);

		gtk_tree_model_get(model, &iter, ID, &whereData, -1);


		// Update the database
		databaseRemoveCat(whereData);
		inventoryModifyCategory("", whereData, "category");
		
		// Write code here to update the taxes and fees database
		// the 0 is the mode, in this case, 0 = edit the item. 1 = remove the item
		//databaseCategoriesUpdate(whereData, NULL, 1, "feeName", FEES_TABLES, "FeesCategories");			
		//databaseCategoriesUpdate(whereData, NULL, 1, "taxGroup", TAXES_TABLES, "TaxGroupsCategories"); 

		// Remove the row from the tree
		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
		gchar *rowtext;
		
		if(gtk_tree_model_get_iter_first(model, &iter)) {
			gtk_tree_model_get(model, &iter, PARENT, &rowtext, -1);
			
			if(!strcmp(rowtext, whereData)) {
				gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PARENT, "0", -1);
			}
		
			g_free(rowtext);	
		}
		
		// Finish off searching the rest of the rows in the store
		while(gtk_tree_model_iter_next(model, &iter)) {	
			gtk_tree_model_get(model, &iter, PARENT, &rowtext, -1);
			
			if(!strcmp(rowtext, whereData)) {
				gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PARENT, "0", -1);
			}
		
			g_free(rowtext);			
		}
									
		g_free(whereData);
	}
}

// Prepares to remove items from the category inventory
static int prepareItemRemove(GtkWidget *widget, intrackCategories *intrackCat) {

	if(intrackCat->selectedCat == NULL) {
		printMessage("ERROR: Select a category.", intrackCat->mainWindow);
		return 1;
	}
	
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->catInventoryTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catInventoryTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	/* Create tree row references to all of the selected rows */
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}
	
	if(references != NULL) {
		/* Remove each of the selected rows pointed to by the row reference */
		g_list_foreach(references, (GFunc) beginItemRemove, intrackCat);
	} else {
		printMessage("ERROR: Select items to remove.", intrackCat->mainWindow);
	}

	/* Refresh the inventory tax group tree */
	//getGroupInventory(NULL, inventory);

	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	return 0;
}

// Prepares to remove categories
static void prepareCatRemoval(GtkWidget *widget, intrackCategories *intrackCat) {

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->catTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	// Create tree row references to all of the selected rows
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	// Remove each of the selected rows pointed to by the row reference
	g_list_foreach(references, (GFunc) beginCatRemove, intrackCat);

	// Refresh the inventory group tree
	//getGroupInventory(NULL, inventory);

	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	g_free(intrackCat->selectedCat);
	intrackCat->selectedCat = NULL;	
	
	g_free(intrackCat->selectedItemCode);
	intrackCat->selectedItemCode = g_strconcat(NULL, NULL);
			
	getCatInventory(intrackCat->catInventoryTree, intrackCat->selectedCat);
	
	//gtk_widget_set_sensitive(intrackCat->deleteCatButton, FALSE);
	gtk_widget_set_sensitive(intrackCat->deleteCatButtonBar, FALSE);
	//gtk_widget_set_sensitive(intrackCat->addCatItems, FALSE);
	gtk_widget_set_sensitive(intrackCat->addCatItemsBar, FALSE);
	//gtk_widget_set_sensitive(intrackCat->deleteCatItems, FALSE);
	gtk_widget_set_sensitive(intrackCat->deleteCatItemsBar, FALSE);		
}

// Changes a inventory items inventory category
int inventoryModifyCategory(gchar *selectedGroup, gchar *data, gchar *where) {

	gchar *query_string;
	int query_state;
	
	/* Open MYSQL connection to the database */
	if(connectToServer() == 1) {
		return 1;
	}

    // Select the database.
    query_string = g_strconcat(mysqlDatabase, NULL);
	query_state = mysql_select_db(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and select database.
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		mysql_close(connection);
		
		return 1;
	}
	
	query_string = g_strconcat("UPDATE `", INVENTORY_TABLES, "` SET category='", selectedGroup, "' WHERE ", where, "='", data, "'", NULL);
	query_state = mysql_query(connection, query_string);
	g_free(query_string);

	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		mysql_close(connection);
		
		return 1;
	}
	
	mysql_close(connection);
	
	return 0;
}

/* Prepare to pull inventory data from the database */
/*
static void getInventory(GtkWidget *widget, intrackCategories *intrackCat) {
	GtkListStore *store;
	
	store = gtk_list_store_new (INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	// Checks to see if a ' is entered, that causes mysql.h to break
	if(g_strrstr(gtk_entry_get_text(GTK_ENTRY(intrackCat->inventorySearchEntry)), "'")) {
		printMessage("ERROR: ' not allowed.", intrackCat->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(intrackCat->inventorySearchEntry), ""); // Clear out the search entry
	}
	// This will search the inventory for the user query if the search entry is greater than 2 characters
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(intrackCat->inventorySearchEntry))) > 2) {
		gchar *searchString;
		
		searchString = g_strconcat(gtk_entry_get_text(GTK_ENTRY(intrackCat->inventorySearchEntry)), NULL);
		
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->barcodeSearch)))
			pullInventory(store, "barcode", searchString, NULL, NULL);
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->nameSearch)))
			pullInventory(store, "name", searchString, NULL, NULL);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->descriptionSearch)))
			pullInventory(store, "description", searchString, NULL, NULL);
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->manufacturerSearch)))
			pullInventory(store, "manufacturer", searchString, NULL, NULL);
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->categorySearch)))
			pullInventory(store, "category", searchString, NULL, NULL);
		
		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(intrackCat->inventorySearchEntry), "");
	}
	// If the search entry is less than 3 characters but greater than 0
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(intrackCat->inventorySearchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(intrackCat->inventorySearchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, intrackCat->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(intrackCat->inventorySearchEntry), "");
	}
	else
		pullInventory(store, NULL, NULL, NULL, NULL); // This pulls all the inventory out of the database
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);
}
*/
/* Pulls inventory data when searching by numbers ie: cost, price and stock */
/*
static void getInventoryNumbers(GtkWidget *widget, intrackCategories *intrackCat) {
	GtkListStore *store;
	
	store = gtk_list_store_new(INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	float min, max;
	
	min = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackCat->searchSpinMin));
	max = gtk_spin_button_get_value(GTK_SPIN_BUTTON(intrackCat->searchSpinMax));
	
	if(max >= min) {
		gchar *minChar, *maxChar;
		minChar = g_strdup_printf("%f", min);
		maxChar = g_strdup_printf("%f", max);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->costSearch)))
			pullInventory(store, "cost", NULL, minChar, maxChar);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->priceSearch)))
			pullInventory(store, "price", NULL, minChar, maxChar);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(intrackCat->stockSearch)))
			pullInventory(store, "stock", NULL, minChar, maxChar);
			
		g_free(minChar);
		g_free(maxChar);
	}
	else
		printMessage(ERROR_SEARCH_NUMBER, intrackCat->mainWindow);

	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);	
}
*/
/* Pulls the inventory data out of the database and stores it into a GtkListStore */
/*
static int pullInventory(GtkListStore *store, gchar *searchWhere, gchar *searchString, gchar *searchNumMin, gchar *searchNumMax) {
	GtkTreeIter 	iter;
	
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;		
	
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

	// Search for the user entered query
	if(searchString != NULL)
		query_string = g_strconcat("SELECT id, barcode, name, description, manufacturer, category, cost, costAverage, price, stock, serialRequired, numberOfSerial, lastSold, duty, countryOrigin FROM `", INVENTORY_TABLES, "`", " WHERE ", searchWhere, " LIKE '%", searchString, "%'", NULL);
	else if(searchNumMin != NULL && searchNumMax != NULL)
		query_string = g_strconcat("SELECT id, barcode, name, description, manufacturer, category, cost, costAverage, price, stock, serialRequired, numberOfSerial, lastSold, duty, countryOrigin FROM `", INVENTORY_TABLES, "`", " WHERE ", searchWhere, " >= ", searchNumMin, " AND ", searchWhere, " <= ", searchNumMax, NULL);
	else	
		query_string = g_strconcat("SELECT id, barcode, name, description, manufacturer, category, cost, costAverage, price, stock, serialRequired, numberOfSerial, lastSold, duty, countryOrigin FROM `", INVENTORY_TABLES, "` LIMIT 0, 1000", NULL);

	mysql_query(connection, query_string);

	//If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	

	while ((row = mysql_fetch_row(result))) {
		gchar *barcode, *name, *description, *manufacturer, *category, *cost, *costAverage, *price, *stock;
		
		for(i = 0; i < num_fields; i++) {
			if(i == 1)
				barcode = g_strconcat(row[i], NULL);
			
			if(i == 2)
				name = g_strconcat(row[i], NULL);
			
			if(i == 3)
				description = g_strconcat(row[i], NULL);
				
			if(i == 4)
				manufacturer = g_strconcat(row[i], NULL);

			if(i == 5)
				category = g_strconcat(row[i], NULL);
				
			if(i == 6)
				cost = g_strconcat(row[i], NULL);
				
			if(i == 7)
				costAverage = g_strdup_printf("%0.2f", getAverageCost(barcode)); //costAverage = g_strconcat(row[i], NULL);
			
			if(i == 8)
				price = g_strconcat(row[i], NULL);
				
			if(i == 9)
				stock = g_strconcat(row[i], NULL);
		}
		
		// Now search the tree store to see if the item already exists in it, we search via the barcode for identification. This prevents duplicates from getting displayed
      	if(searchString != NULL || (searchNumMin != NULL && searchNumMax != NULL)) {
			gchar *rowtext;
			gboolean foundItem = FALSE;
			
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
				gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, 0, &rowtext, -1); // the barcode column
		
				if(rowtext != NULL) {
					// Found item
					if(!strcmp(rowtext, barcode))
						foundItem = TRUE;
						
					g_free(rowtext);
				}
				
				// Finish off searching the rest of the rows in the store
				while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)) {			
					gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, 0, &rowtext, -1); // the barcode column

					if(rowtext != NULL) {
						// Found item
						if(!strcmp(rowtext, barcode)) {
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
					
				gtk_list_store_set (store, &iter, BARCODE, barcode, NAME, name, DESCRIPTION, description, MANUFACTURER, manufacturer, CATEGORY, category, COST, cost, COSTAVG, costAverage, PRICE, price, STOCK, stock, -1);
			}			
		}
		else {
			// This stores all inventory into the tree
			gtk_list_store_append (store, &iter);
			
			gtk_list_store_set (store, &iter, BARCODE, barcode, NAME, name, DESCRIPTION, description, MANUFACTURER, manufacturer, CATEGORY, category, COST, cost, COSTAVG, costAverage, PRICE, price, STOCK, stock, -1);
		}
		
		g_free(barcode);
		g_free(name);
		g_free(description);
		g_free(manufacturer);
		g_free(category);
		g_free(cost);
		g_free(costAverage);
		g_free(price);
		g_free(stock);
	}	

	g_free(query_string);	
	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}
*/
/* Gets the average cost of the item from the database */
/*
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
*/

/* Creates a new category */
static void createCatWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	
	intrackCatEntry *catEntry;
	catEntry = (intrackCatEntry*) g_malloc (sizeof (intrackCatEntry)); // Allocate memory for the data.	
	
	// Create and setup the new input popup window.
	catEntry->inputWindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	
	//gtk_window_set_default_size(GTK_WINDOW(taxEntry->inputWindow), 320, 220);
	gtk_widget_set_size_request(catEntry->inputWindow, 300, 110);
	
	gtk_window_set_title(GTK_WINDOW(catEntry->inputWindow), "Create New Category");
	gtk_container_set_border_width(GTK_CONTAINER(catEntry->inputWindow), 10);
	
	// Set the new inputWindow so it is above the original parent window of the program.
	gtk_window_set_transient_for(GTK_WINDOW(catEntry->inputWindow), GTK_WINDOW(intrackCat->mainWindow));

	// Create a table to layout and organize the text and entry fields.
	catEntry->table = gtk_table_new(4, 2, FALSE);
	gtk_container_add(GTK_CONTAINER(catEntry->inputWindow), catEntry->table);
	
	gtk_window_set_position(GTK_WINDOW(catEntry->inputWindow), GTK_WIN_POS_CENTER);    

	catEntry->inputLabel = gtk_label_new("Category Name:");
	
	gtk_table_attach(GTK_TABLE(catEntry->table), catEntry->inputLabel, 0, 1, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	catEntry->inputEntry = gtk_entry_new();

	catEntry->sendButton = gtk_button_new_with_label("Ok");
	gtk_widget_set_size_request(catEntry->sendButton, 80, 30 );
		
	catEntry->cancelButton = gtk_button_new_with_label("Cancel");
	gtk_widget_set_size_request(catEntry->cancelButton, 80, 30 );	
	
	gtk_table_set_homogeneous(GTK_TABLE(catEntry->table), FALSE);

	gtk_table_attach(GTK_TABLE(catEntry->table), catEntry->inputEntry, 1, 2, 0, 1, GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 5, 5);

	gtk_table_attach(GTK_TABLE(catEntry->table), catEntry->sendButton, 1, 2, 1, 2,  GTK_SHRINK, GTK_SHRINK, 1, 5);
	gtk_table_attach(GTK_TABLE(catEntry->table), catEntry->cancelButton, 0, 1, 1, 2, GTK_SHRINK, GTK_SHRINK, 1, 5);

	// Set the positions of my buttons so they are spaced and look good.
	gtk_widget_set_uposition(catEntry->sendButton, 200, 60);
	gtk_widget_set_uposition(catEntry->cancelButton, 110, 60);
	
	/* Set the windows so windows below can not be grabbed, and set it not resizable and hide the close and maximize buttons */
	gtk_window_set_modal(GTK_WINDOW(catEntry->inputWindow), TRUE);
	gtk_window_set_resizable(GTK_WINDOW(catEntry->inputWindow), FALSE);
	gtk_window_set_deletable(GTK_WINDOW(catEntry->inputWindow), FALSE);	

	gtk_widget_show_all(catEntry->inputWindow);

	// Close the window (destroy the widget)
	g_signal_connect(G_OBJECT(catEntry->cancelButton), "clicked", G_CALLBACK(destroyWidget), catEntry->inputWindow);

	g_signal_connect(G_OBJECT(catEntry->sendButton), "clicked", G_CALLBACK(databaseCreateCat), catEntry);
	g_signal_connect(G_OBJECT(catEntry->sendButton), "clicked", G_CALLBACK(getCategories), intrackCat); /* Refresh the cat tree */
	g_signal_connect(G_OBJECT(catEntry->sendButton), "clicked", G_CALLBACK(catAddSensitive), intrackCat);
	g_signal_connect(G_OBJECT(catEntry->sendButton), "clicked", G_CALLBACK(destroyWidget), catEntry->inputWindow);

	g_signal_connect(G_OBJECT(catEntry->inputWindow), "destroy", G_CALLBACK(freeMemoryCatWindow), catEntry);

	gtk_widget_show(catEntry->inputWindow);
}

static void catAddSensitive(GtkWidget *widget, intrackCategories *intrackCat) {
	//gtk_widget_set_sensitive(intrackCat->deleteCatButton, FALSE);
	gtk_widget_set_sensitive(intrackCat->deleteCatButtonBar, FALSE);
	//gtk_widget_set_sensitive(intrackCat->addCatItems, FALSE);
	gtk_widget_set_sensitive(intrackCat->addCatItemsBar, FALSE);
	//gtk_widget_set_sensitive(intrackCat->deleteCatItems, FALSE);
	gtk_widget_set_sensitive(intrackCat->deleteCatItemsBar, FALSE);	
}

/* Creates a new category in the database */
static int databaseCreateCat(GtkWidget *widget, intrackCatEntry *catEntry) {
	
	gchar *inputData;
	inputData = g_strconcat(gtk_entry_get_text(GTK_ENTRY(catEntry->inputEntry)), NULL);
	
	if(g_strrstr(inputData, "'")) {
		printMessage("ERROR: ' not allowed.", NULL);
		g_free(inputData);
		return 1;
	}	
	
	/* Now begin category creation */
	gchar *query_string;
	int query_state;

	if(connectToServer() == 1)
		return 1;
	
    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	
			
	query_string = g_strconcat("INSERT INTO `", CATEGORY_TABLES, "` (display_name) VALUES ('", inputData, "')", NULL);
	
	query_state=mysql_query(connection, query_string);
	
	// Failed to query the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage("ERROR: Error connecting to server, check query syntax", NULL);
				
		g_free(query_string);	
		g_free(inputData);

		mysql_close(connection);
		return 1;
	}		

	g_free(query_string);
	g_free(inputData);
	
	mysql_close(connection);		
	
	return 0;
}

// Removes a category in the database
static int databaseRemoveCat(gchar *whereData) {
	
	// Now begin category creation
	gchar *query_string;
	int query_state;

	if(connectToServer() == 1)
		return 1;
	
    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}		
		
	query_string = g_strconcat("DELETE FROM `", CATEGORY_TABLES, "` WHERE id='", whereData, "'", NULL);

	query_state=mysql_query(connection, query_string);
	
	// Failed to query the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage("ERROR: Error connecting to server, check query syntax", NULL);
				
		g_free(query_string);	
		mysql_close(connection);
		
		return 1;
	}

	g_free(query_string);
	
	query_string = g_strconcat("UPDATE `", CATEGORY_TABLES, "` SET parent_id='0' WHERE parent_id='", whereData, "'", NULL);
	
	query_state=mysql_query(connection, query_string);
	
	// Failed to query the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage("ERROR: Error connecting to server, check query syntax", NULL);
				
		g_free(query_string);	
		mysql_close(connection);
		
		return 1;
	}	
	
	g_free(query_string);	
	
	mysql_close(connection);		
	
	return 0;
}

/* Checks input validation and if the category exists or not */
static int checkCatExist(gchar *inputData) {

	/* Do a check to make sure the new tax name is valid */
	if(strlen(inputData) < 1) {
		printMessage("ERROR: New category name is too short", NULL);
		return 1;
	}

	/* Now begin searching the category names */
	gchar *query_string;
	int query_state;
	int num_fields;
	int i;	

	if(connectToServer() == 1)
		return 1;
		
    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}			
		
	query_string = g_strconcat("SELECT category FROM `", CATEGORY_TABLES, "` WHERE category='", inputData, "'", NULL);
	query_state=mysql_query(connection, query_string);
	
	/* Failed to query the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage("ERROR: Error connecting to server, check query syntax", NULL);
				
		g_free(query_string);	
		mysql_close(connection);
		return 1;
	}		
		
	/* Store the query results into reportResult */
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);		
	
	while ( ( row = mysql_fetch_row(result)) ) {
		for(i = 0; i < num_fields; i++) {

			/* If the returned result has 1 or more characters, it means it found a taxgroup name already exists, then exit and return */
			if(strlen(row[i]) > 0) {
				printMessage("ERROR: Category already exists", NULL);
				g_free(query_string);
				
				mysql_free_result(result);
				mysql_close(connection);
				return 1;
			}
		}
	}
	
	g_free(query_string);
	mysql_free_result(result);
	mysql_close(connection);		
	
	return 0;
}

static void freeMemoryCatWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	clear_tree_store(intrackCat);
	g_free(intrackCat);	
}

// Clears the tree store, quicker to erase from memory when the tree is empty.
static void clear_tree_store(intrackCategories *intrackCat) {
	GtkTreeStore	*store;
	GtkTreeIter 	iter;
	
	store = gtk_tree_store_new(CAT_INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter);

	gtk_tree_store_append(store, &iter, NULL);
			
	gtk_tree_store_set(store, &iter, CAT_ID, "", CAT_BARCODE, "", CAT_DESCRIPTION, "", CAT_MANUFACTURER, "", CAT_EXTRAINFO, "", -1);
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->catInventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);		
}

// Prepare to pull category data from the database
static void getCategories(GtkWidget *widget, intrackCategories *intrackCat) {
	GtkTreeStore *store;

	store = gtk_tree_store_new (CAT_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

	pullCategories(store); // This pulls all the categories out of the database
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->catTree), GTK_TREE_MODEL(store));
	
	g_object_unref(store);
}

// Pulls the category data out of the database and stores it into a GtkListStore
static int pullCategories(GtkTreeStore *store) {
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;		
	
	// Open MYSQL connection to the database
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//	RUN 2 QUERIES. ONE FOR ROOT parent_id then another set for childs.
//	ORDER AND SORT/GROUP BY IDS
// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//query_string = g_strconcat("SELECT * FROM `", CATEGORY_TABLES, "` WHERE parent_id = 0 ORDER BY id", NULL);
	query_string = g_strconcat("SELECT a.id, a.parent_id, a.display_name, a.order, b.label, a.section FROM `", CATEGORY_TABLES, "` a left join `", CATEGORY_SECTION, "` b  on a.section = b.id ORDER BY ID", NULL);
	
	mysql_query(connection, query_string);
	
	// If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
	
	//GtkTreeIter		topiter;
	GtkTreeIter		tempiter;
	//GtkTreeStore 	*templist;

	//templist = gtk_tree_store_new (CAT_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	int totalcategories = 0;

	while ((row = mysql_fetch_row(result))) {
		gchar *id, *parent_id, *display_name, *order, *section, *section_id;
		
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				id = g_strdup(row[i]);
			
			if(i == 1)
				parent_id = g_strdup(row[i]);
			
			if(i == 2)
				display_name = g_strdup(row[i]);
				
			if(i == 3)
				order = g_strdup(row[i]);
				
			if(i == 4)
				section = g_strdup(row[i]);
				
			if(i == 5)
				section_id = g_strdup(row[i]);				
		}
		// http://zetcode.com/tutorials/gtktutorial/gtktreeview/
		// https://developer.gnome.org/gtk3/stable/GtkTreeStore.html#gtk-tree-store-append
		gtk_tree_store_append (store, &tempiter, NULL);
		gtk_tree_store_set (store, &tempiter, ID, id, CATNAME, display_name, PARENT, parent_id, ORDER, order, SECTION, section, SECTION_ID, section_id, -1);
		
		g_free(id), g_free(parent_id), g_free(display_name), g_free(order), g_free(section), g_free(section_id);
		
		totalcategories++;
	}
	g_free(query_string);
	mysql_free_result(result);
	/*
	// Now search the temp store list to look for parents and childs
	int i = 0;
	GtkTreeIter		childiter;
	gboolean 		foundChild;
	gchar 			*parent_temp_id;
	
	while(i <= totalcategories) {
		if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(templist), &tempiter)) {
			gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, PARENT, &parent_temp_id, -1);
			
			if(parent_temp_id != NULL) {
				// Found a child
				if(atoi(parent_temp_id) == i) {
					gchar *id_temp, *display_name_temp, *parent_id_temp, *order_temp, *section_temp;
					
					gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, ID, &id_temp, -1);
					gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, CATNAME, &display_name_temp, -1);
					gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, PARENT, &parent_id_temp, -1);
					gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, ORDER, &order_temp, -1);
					gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, SECTION, &section_temp, -1);
					
					gtk_tree_store_append (store, &childiter, &topiter);
					gtk_tree_store_set (store, &childiter, ID, id_temp, CATNAME, display_name_temp, PARENT, parent_id_temp, ORDER, order_temp, SECTION, section_temp, -1);
					
					// wipe out the current temp entry so it's removed from the list from future checks.
					gtk_tree_store_set (templist, &tempiter, ID, "", CATNAME, "", PARENT, "", ORDER, "", SECTION, "", -1);
					
					g_free(id_temp), g_free(display_name_temp), g_free(parent_id_temp), g_free(order_temp), g_free(section_temp);
				}
			
				g_free(parent_temp_id);
			}
					
				// Finish off searching the rest of the rows
				while(gtk_tree_model_iter_next(GTK_TREE_MODEL(templist), &tempiter)) {
					gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, PARENT, &parent_temp_id, -1); 

					if(parent_temp_id != NULL) {
						// Found a child
						if(atoi(parent_temp_id) == i) {
							gchar *id_temp, *display_name_temp, *parent_id_temp, *order_temp, *section_temp;
							
							gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, ID, &id_temp, -1);
							gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, CATNAME, &display_name_temp, -1);
							gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, PARENT, &parent_id_temp, -1);
							gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, ORDER, &order_temp, -1);
							gtk_tree_model_get(GTK_TREE_MODEL (templist), &tempiter, SECTION, &section_temp, -1);
							
							gtk_tree_store_append (store, &childiter, &topiter);
							gtk_tree_store_set (store, &childiter, ID, id_temp, CATNAME, display_name_temp, PARENT, parent_id_temp, ORDER, order_temp, SECTION, section_temp, -1);
							
							g_free(id_temp), g_free(display_name_temp), g_free(parent_id_temp), g_free(order_temp), g_free(section_temp);
						}

						g_free(parent_temp_id);
					}
				}
		}
		
		i++;
	}
	g_object_unref(templist);
	*/
	
	/*
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	//	run thru the childs
	GtkTreeIter		childiter;
	GtkTreeIter		thirditer;
	
	gchar *query_child;
	query_child = g_strconcat("SELECT * FROM `", CATEGORY_TABLES, "` WHERE parent_id > 0 ORDER BY id", NULL);
	
	mysql_query(connection, query_child);

	// If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
	
	while ((row = mysql_fetch_row(result))) {
		gchar *id, *parent_id, *display_name, *order, *section;
		
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				id = g_strdup(row[i]);
			
			if(i == 1)
				parent_id = g_strdup(row[i]);
			
			if(i == 2)
				display_name = g_strdup(row[i]);
				
			if(i == 3)
				order = g_strdup(row[i]);
				
			if(i == 4)
				section = g_strdup(row[i]);
		}
		
			// Now search the tree store to look for parents and childs
			gchar *rowtext;
			gboolean foundItem = FALSE;
			gboolean foundItemChild = FALSE;
			
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &topiter)) {
				gtk_tree_model_get(GTK_TREE_MODEL (store), &topiter, ID, &rowtext, -1); // the id column
		
				if(rowtext != NULL) {
					g_print("topiter:%s\n", rowtext);
					// Found item
					if(!strcmp(rowtext, parent_id)) {
						foundItem = TRUE;
					}
		
					g_free(rowtext);
				}
				
				if(foundItem == FALSE) {
					// Finish off searching the rest of the rows in the store
					while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &topiter)) {			
						gtk_tree_model_get(GTK_TREE_MODEL (store), &topiter, ID, &rowtext, -1); // the id column

						if(rowtext != NULL) {
							g_print("topiter:%s\n", rowtext);
							// Found item
							if(!strcmp(rowtext, parent_id)) {
								foundItem = TRUE;
								g_free(rowtext);
								break;
							}
							
							g_free(rowtext);
						}
					}
				}
			}
			
			if(foundItem == FALSE) {
				if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &childiter)) {
					gtk_tree_model_get(GTK_TREE_MODEL (store), &childiter, ID, &rowtext, -1); // the id column

					if(rowtext != NULL) {
						g_print("childiter: %s\n", rowtext);
						// Found item
						if(!strcmp(rowtext, parent_id)) {
							foundItemChild = TRUE;
						}
			
						g_free(rowtext);
					}
					
					if(foundItemChild == FALSE) {
						// Finish off searching the rest of the rows in the store
						while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &childiter)) {		
							gtk_tree_model_get(GTK_TREE_MODEL (store), &childiter, ID, &rowtext, -1); // the id column
							
							if(rowtext != NULL) {
								g_print("childiter: %s\n", rowtext);
								// Found item
								if(!strcmp(rowtext, parent_id)) {
									foundItemChild = TRUE;
									g_free(rowtext);
									break;
								}
								
								g_free(rowtext);
							}
						}
					}
				}				
			}
		
		if(foundItem == TRUE) {
			gtk_tree_store_append (store, &childiter, &topiter);
			gtk_tree_store_set (store, &childiter, ID, id, CATNAME, display_name, PARENT, parent_id, ORDER, order, SECTION, section, -1);
			g_print("child: %s\n", display_name);
		}
		else if(foundItemChild == TRUE) {
			gtk_tree_store_append (store, &thirditer, &childiter);
			gtk_tree_store_set (store, &thirditer, ID, id, CATNAME, display_name, PARENT, parent_id, ORDER, order, SECTION, section, -1);
			g_print("third child: %s\n", display_name);
		}
		else {
			gtk_tree_store_append (store, &topiter, NULL);
			gtk_tree_store_set (store, &topiter, ID, id, CATNAME, display_name, PARENT, parent_id, ORDER, order, SECTION, section, -1);
			g_print("parent: %s\n", display_name);
		}
		
		g_free(id), g_free(parent_id), g_free(display_name), g_free(order), g_free(section);		
	}
	
	g_free(query_child);
	mysql_free_result(result);
	mysql_close(connection);
	*/
	
	return 0;
}

// Loads the inventory of the selected category
int getCatInventory(GtkWidget *treeview, gchar *category) {
	
	GtkTreeStore 	*store;
	GtkTreeIter 	iter;
	
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;		
	
	store = gtk_tree_store_new (CAT_INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	// If no category selected, return a empty tree store.
	if(category == NULL) {
		gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
		g_object_unref (store);		
		
		return 1;
	}	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	query_string = g_strconcat("SELECT id, partNo, description, manufacturer, extraInfo FROM `", INVENTORY_TABLES, "` WHERE category='", category, "'", NULL);
	
	mysql_query(connection, query_string);

	// If the connection is successful and the query returns a result then the next step is to display those results:
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);
	
	while ( ( row = mysql_fetch_row(result)) ) {
		gchar *id, *barcode, *description, *manufacturer, *extrainfo;

		gtk_tree_store_append (store, &iter, NULL);
		
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
				extrainfo = g_strdup(row[i]);								
		}
      
		gtk_tree_store_set (store, &iter, CAT_ID, id, CAT_BARCODE, barcode, CAT_DESCRIPTION, description, CAT_MANUFACTURER, manufacturer, CAT_EXTRAINFO, extrainfo, -1);

		g_free(id), g_free(barcode), g_free(description), g_free(manufacturer), g_free(extrainfo);
	}
	
	gtk_tree_view_set_model (GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
	g_object_unref (store);

	mysql_free_result(result);
	mysql_close(connection);
	
	g_free(query_string);	
		
	return 0;
}

// Removes the category from the category database
static int databaseCategoriesRemove(gchar *whereData, gchar *catNameTable, gchar *databaseName) {
	
	MYSQL *catConnection, catMysql;

	mysql_init(&catMysql);

	catConnection = mysql_real_connect(&catMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	if (catConnection == NULL) {
		printf(mysql_error(catConnection), "%d\n");

		printMessage(ERROR_NETWORK, NULL);
		mysql_close(catConnection);
		checkNetworkConnection();
		return 1;
	}
	
	gchar *query_string;
	int query_state;

	// Select the database.
	query_string = g_strconcat(mysqlDatabase, databaseName, NULL);
	query_state = mysql_select_db(catConnection, query_string);	
	
	g_free(query_string);
	
	query_string = g_strconcat("DELETE FROM `", catNameTable, "` WHERE productCat='", whereData, "'", NULL);
	
	query_state = mysql_query(catConnection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(catConnection), "%d\n");
	}
		
	g_free(query_string);

	mysql_close(catConnection);	
	
	return 0;
}

// Makes changes to the categories.
static int databaseCategoriesEdit(gchar *whereData, gchar *newText, gchar *catNameTable, gchar *databaseName) {
	
	MYSQL *catConnection, catMysql;

	mysql_init(&catMysql);

	catConnection = mysql_real_connect(&catMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	if(catConnection == NULL) {
		printf(mysql_error(catConnection), "%d\n");

		printMessage(ERROR_NETWORK, NULL);
		mysql_close(catConnection);
		checkNetworkConnection();
		return 1;
	}
	
	gchar *query_string;
	int query_state;

	// Select the database.
	query_string = g_strconcat(mysqlDatabase, databaseName, NULL);
	query_state = mysql_select_db(catConnection, query_string);	
	
	g_free(query_string);
	
	query_string = g_strconcat("UPDATE `", catNameTable, "` SET productCat='", newText, "' WHERE productCat='", whereData, "'" , NULL);
	query_state = mysql_query(catConnection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(catConnection), "%d\n");
	}
		
	g_free(query_string);

	mysql_close(catConnection);	
	
	return 0;
}

// Update database categories.
static int databaseCategoriesUpdate(gchar *whereData, gchar *newText, int mode, gchar *selectData, gchar *tables, gchar *databaseName) {

	MYSQL *catConnection, catMysql;
	MYSQL_RES *catResult;
	MYSQL_ROW catRow;

	mysql_init(&catMysql);

	catConnection = mysql_real_connect(&catMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	if (catConnection == NULL) {
		printf(mysql_error(catConnection), "%d\n");

		printMessage(ERROR_NETWORK, NULL);
		mysql_close(catConnection);
		checkNetworkConnection();
		return 1;
	}

	gchar *query_string;
	int query_state;
	int num_fields;
	int i;		

	// Select the database.
	query_string = g_strconcat(mysqlDatabase, NULL);
	query_state = mysql_select_db(catConnection, query_string);
	
	g_free(query_string);
	query_string = g_strconcat("SELECT ", selectData, " FROM `", tables, "`", NULL);
	query_state = mysql_query(catConnection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(catConnection), "%d\n");
	}

	catResult = mysql_store_result(catConnection);
	num_fields = mysql_num_fields(catResult);
	
	// Run through all the categories
	while((catRow = mysql_fetch_row(catResult)) ) {
		for(i = 0; i < num_fields; i++) {
			// The Cat Name
			if(i == 0) {
				if(mode == 0)
					databaseCategoriesEdit(whereData, newText, catRow[i], databaseName);
				
				if(mode == 1)
					databaseCategoriesRemove(whereData, catRow[i], databaseName);
			}
		}
	}

	g_free(query_string);

	mysql_free_result(catResult);
	mysql_close(catConnection);	
	
	return 0;
}

// Quick category change via category_popup.c
static void cellClickedParent(GtkCellRendererText *render, gchar *path, gchar *newText, intrackCategories *intrackCat) {
	
    categoryPopup(intrackCat->mainWindow, intrackCat->selectedCat, intrackCat->catTree, FALSE); // -> category_popup.c
}

// Quick category change via section_popup.c
static void cellClickedSection(GtkCellRendererText *render, gchar *path, gchar *newText, intrackCategories *intrackCat) {
	
    sectionPopup(intrackCat->mainWindow, intrackCat->selectedCat, intrackCat->catTree); // -> section_popup.c
}

// Edits the name of the category
static int cellClickedCat(GtkCellRendererText *render, gchar *path, gchar *newText, intrackCategories *intrackCat) {
	
	// Checks to see if input is valid
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);
    
    // If the new entered text is the same as the old, then just return back
	/*
	if(!strcmp(whereData, newText)) {
		g_free(whereData);
		return 1;
	}
	*/
	
    // Checks to see if the category already exits. We do not want duplicates now 
    /*
	if(checkCatExist(newText)) {
		g_free(whereData);
		return 1;
	}
	*/
	
	// Update the database
	categoryUpdateName(newText, whereData);
	//inventoryModifyCategory(newText, whereData, "category");
	
	/* Write code here to update the taxes and fees fees */
	/* the 0 is the mode, in this case, 0 = edit the item. 1 = remove the item */	
	//databaseCategoriesUpdate(whereData, newText, 0, "feeName", FEES_TABLES, "FeesCategories");	
	//databaseCategoriesUpdate(whereData, newText, 0, "taxGroup", TAXES_TABLES, "TaxGroupsCategories");	
	
	
	/* Update the tree cell with the new data */
	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, CATNAME, newText, -1);

    g_free(whereData);
    
    g_free(intrackCat->selectedCat);
    intrackCat->selectedCat = g_strconcat(newText, NULL);
	
	return 0;
}

// Updates the category name in the database
static int categoryUpdateName(gchar *newText, gchar *whereData) {
	
	gchar *query_string;
	int query_state;		
	
	// Open MYSQL connection to the database
	if(connectToServer() == 1) {
		return 1;
	}

    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	
	
	query_string = g_strconcat("UPDATE `", CATEGORY_TABLES, "` SET display_name='", newText, "' WHERE id='", whereData, "'", NULL);

	query_state = mysql_query(connection, query_string);

	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}
	
	g_free(query_string);
	mysql_close(connection);
	
	return 0;	
	
}

// Updates the category data from category_popup.c
int categoryUpdate(gchar *query_update) {
	
	gchar *query_string;
	int query_state;		
	
	// Open MYSQL connection to the database
	if(connectToServer() == 1) {
		return 1;
	}

    // Select the database.
    query_string = g_strdup(CATEGORY_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	
	
	//query_string = g_strconcat("UPDATE `", CATEGORY_TABLES, "` SET parent_id='", newText, "', section ='", newSection, "' WHERE id='", whereData, "'", NULL);

	query_state = mysql_query(connection, query_update);
	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		//g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}
	
	//g_free(query_string);
	mysql_close(connection);
	
	return 0;	
	
}

// Mouse click event on the trees
static gboolean cat_button_press(GtkWidget *widget, GdkEventButton *ev, intrackCategories *intrackCat) {
	
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeIter iter;
    
    gchar *rowtext, *text;
    
	// if there's no path where the click occurred
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(intrackCat->catTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
        return FALSE;    // return FALSE to allow the event to continue
	}
	
	g_free(intrackCat->selectedItemCode);
	intrackCat->selectedItemCode = g_strconcat(NULL, NULL);	
	
	int previouslySelected = 0;
    // Left mouse button click
    switch(ev->button)
    {
        // Left Mouse Button Click
        case 1: // 1 = left click
				model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catTree));
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, 0, &rowtext, -1);
				
				text = g_strdup_printf("%s", rowtext);
				
				if(intrackCat->selectedCat != NULL) {
					if(!strcmp(text, intrackCat->selectedCat))
						previouslySelected++;
				}
				
				g_free(intrackCat->selectedCat);	
				intrackCat->selectedCat = g_strdup_printf("%s", rowtext);
				
				//gtk_widget_set_sensitive(intrackCat->deleteCatButton, TRUE);
				gtk_widget_set_sensitive(intrackCat->deleteCatButtonBar, TRUE);
				//gtk_widget_set_sensitive(intrackCat->addCatItems, TRUE);
				gtk_widget_set_sensitive(intrackCat->addCatItemsBar, TRUE);
				//gtk_widget_set_sensitive(intrackCat->deleteCatItems, TRUE);
				gtk_widget_set_sensitive(intrackCat->deleteCatItemsBar, TRUE);				
				
				if(previouslySelected < 1)
					getCatInventory(intrackCat->catInventoryTree, text);
				
				g_free(rowtext);
				g_free(text);
            break;
        // Right Mouse Button Click
        case 3: // 3 = right click
				model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catTree));
				gtk_tree_model_get_iter(model, &iter, path);
				gtk_tree_model_get(model, &iter, 0, &rowtext, -1);
				
				text = g_strdup_printf("%s", rowtext);
				
				if(intrackCat->selectedCat != NULL) {
					if(!strcmp(text, intrackCat->selectedCat))
						previouslySelected++;
				}
									
				g_free(intrackCat->selectedCat);	
				intrackCat->selectedCat = g_strdup_printf("%s", rowtext);
				
				// gtk_widget_set_sensitive(intrackCat->deleteCatButton, TRUE);
				gtk_widget_set_sensitive(intrackCat->deleteCatButtonBar, TRUE);
				// gtk_widget_set_sensitive(intrackCat->addCatItems, TRUE);
				gtk_widget_set_sensitive(intrackCat->addCatItemsBar, TRUE);
				// gtk_widget_set_sensitive(intrackCat->deleteCatItems, TRUE);
				gtk_widget_set_sensitive(intrackCat->deleteCatItemsBar, TRUE);			
				
				if(previouslySelected < 1)
					getCatInventory(intrackCat->catInventoryTree, text);

				g_free(rowtext);
				g_free(text);
                    
				// gtk_menu_popup(GTK_MENU(intrackTaxes->taxMenuPopUp), NULL, NULL, NULL, NULL, 3, ev->time);
				
            break;
    }

    // free our path
    gtk_tree_path_free(path);
	
    return FALSE;    // allow it to continue
}

// Called by treeview_key_press(). It is delayed by 1 millisecond so we can accurately get the proper selected row from the tree
static gboolean treeview_key_timer(gpointer data) {
    
    intrackCategories *intrackCat = (intrackCategories *)data;
    
    GtkTreeModel *model;
    GtkTreePath *path;
    GtkTreeSelection *selection;
    GtkTreeIter iter;
    gboolean pathSelected;
    
    gchar *rowtext;
    
    // Need to pull the current selected row from the treeview
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(intrackCat->catTree));

	// Now need to pull the path and iter from the selected row
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);
		pathSelected = TRUE;
	}
	else {
		pathSelected = FALSE;
		
		return TRUE;
	}	
	
	int previouslySelected = 0;
	
	if(pathSelected) {

		if(!gtk_tree_model_get_iter(model, &iter, path)) {
			// If the iter is invalid, it means the path is invalid, so go back to the prev path and grab the iter again
			gtk_tree_path_prev(path);
			gtk_tree_model_get_iter(model, &iter, path);
		}
			
		gtk_tree_model_get(model, &iter, 0, &rowtext, -1); // id is getting pulled. this was at 1 before. why??
			
		g_free(intrackCat->selectedItemCode);
		intrackCat->selectedItemCode = g_strconcat(NULL, NULL);
		
		if(intrackCat->selectedCat != NULL) {
			if(!strcmp(rowtext, intrackCat->selectedCat))
				previouslySelected++;
		}
			
		g_free(intrackCat->selectedCat);			
		intrackCat->selectedCat = g_strdup_printf("%s", rowtext); // store which selected category
		
		//gtk_widget_set_sensitive(intrackCat->deleteCatButton, TRUE);
		gtk_widget_set_sensitive(intrackCat->deleteCatButtonBar, TRUE);
		//gtk_widget_set_sensitive(intrackCat->addCatItems, TRUE);
		gtk_widget_set_sensitive(intrackCat->addCatItemsBar, TRUE);
		//gtk_widget_set_sensitive(intrackCat->deleteCatItems, TRUE);
		gtk_widget_set_sensitive(intrackCat->deleteCatItemsBar, TRUE);		
		
		// No need to reload a previously selected category
		if(previouslySelected < 1)	
			getCatInventory(intrackCat->catInventoryTree, rowtext);

		g_free(rowtext);
	}
	
	
	if(pathSelected) {
		// free our path
		gtk_tree_path_free(path);
	}
		
	return FALSE;
}

// Keyboard key press event on the category tree
static gboolean cat_key_press(GtkWidget *widget, GdkEventKey *ev, intrackCategories *intrackCat) {
	
    switch(ev->keyval)
    {
        case GDK_Up:
				// Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection.
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;

        case GDK_Down:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);	
            break;

        case GDK_KP_Down:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);	
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);	
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;  
            
        case GDK_End:
				g_timeout_add (1, treeview_key_timer, (gpointer) intrackCat);		
            break;
            
        case GDK_Delete:
				//deleteTaxWindow(NULL, intrackTaxes);
			break;
			
		case GDK_KP_Delete:
				//deleteTaxWindow(NULL, intrackTaxes);
			break;
    }
	
    return FALSE;    // allow it to continue
}

/* Create the tree view and columns */
/*
static void setupInventoryTree(intrackCategories *intrackCat) { 
	GtkListStore *store;
	
	store = gtk_list_store_new (INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Barcode", renderer, "text", BARCODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE); 
	gtk_tree_view_column_set_sort_column_id (column, BARCODE); 
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedBarcode), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", NAME, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE); 
	gtk_tree_view_column_set_sort_column_id (column, NAME); 
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedName), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", DESCRIPTION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE); 
	gtk_tree_view_column_set_sort_column_id (column, DESCRIPTION); 
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDescription), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Manufacturer", renderer, "text", MANUFACTURER, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE); 
	gtk_tree_view_column_set_sort_column_id (column, MANUFACTURER); 
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedManufacturer), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Category", renderer, "text", CATEGORY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE); 
	gtk_tree_view_column_set_sort_column_id (column, CATEGORY); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Cost", renderer, "text", COST, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COST); 
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Avg Cost", renderer, "text", COSTAVG, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COSTAVG); 
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", PRICE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PRICE);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedPrice), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Stock", renderer, "text", STOCK, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, STOCK);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedStock), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->inventoryTree), column);	
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->inventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);		
}
*/
static void setupCatInventoryTree(intrackCategories *intrackCat) {
	GtkListStore *store;
	
	store = gtk_list_store_new (CAT_INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
		
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catInventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Barcode", renderer, "text", CAT_BARCODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CAT_BARCODE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catInventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", CAT_DESCRIPTION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CAT_BARCODE); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catInventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Manufacturer", renderer, "text", CAT_MANUFACTURER, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CAT_MANUFACTURER); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catInventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Extra Info", renderer, "text", CAT_EXTRAINFO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CAT_EXTRAINFO); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catInventoryTree), column);			
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(intrackCat->catInventoryTree), GTK_TREE_MODEL(store));
	g_object_unref(store);
}

static void setupCategoryTree(intrackCategories *intrackCat) {
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	//gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Category Name", renderer, "text", CATNAME, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CATNAME); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCat), (gpointer) intrackCat);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Parent", renderer, "text", PARENT, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PARENT); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "editing-started", G_CALLBACK (cellClickedParent), (gpointer) intrackCat);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Order", renderer, "text", ORDER, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, ORDER); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Section", renderer, "text", SECTION, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SECTION);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "editing-started", G_CALLBACK (cellClickedSection), (gpointer) intrackCat);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Section ID", renderer, "text", SECTION_ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	//gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, SECTION_ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (intrackCat->catTree), column);			
	
}

/* Mouse click event on the tree rules */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackCategories *intrackCat) {

    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(intrackCat->catInventoryTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
		gtk_widget_set_sensitive(intrackCat->invenMenuView, FALSE);
        return FALSE;
	}

    /* LMB */
    switch(ev->button)
    {
        case 1: /* 1 = left click */
            break;

        case 3: /* 3 = right click */
				gtk_widget_set_sensitive(intrackCat->invenMenuView, TRUE);
				gtk_menu_popup(GTK_MENU(intrackCat->invenMenu), NULL, NULL, NULL, NULL, 3, ev->time);
            break;
    }
    
	/* Keep track of the selected item */
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gchar *rowBarcode;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(intrackCat->catInventoryTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, CAT_ID, &rowBarcode, -1);
        
    g_free(intrackCat->selectedItemCode);
    intrackCat->selectedItemCode = g_strconcat(rowBarcode, NULL);
    g_free(rowBarcode);
    
    /* free our path */
    gtk_tree_path_free(path);  

    return FALSE;	
}

/* A widget window to ask for confirmation upon deletion */
static int deleteCatWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	
	if(intrackCat->selectedCat == NULL) {
		printMessage("ERROR: Select a category", intrackCat->mainWindow);
		return 1;
	}		
		
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(intrackCat->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Delete %s ?", intrackCat->selectedCat);
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Delete Category");
	widgetDialog = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (widgetDialog) {
		case GTK_RESPONSE_YES:
			prepareCatRemoval(NULL, intrackCat);
			break;
		case GTK_RESPONSE_NO:
			break;
	}

	return 0;
}

/* A widget window to ask for confirmation upon deletion */
static int removeItemsWindow(GtkWidget *widget, intrackCategories *intrackCat) {
	
	if(intrackCat->selectedCat == NULL) {
		printMessage("ERROR: Select a category", intrackCat->mainWindow);
		return 1;
	}	
	
	if(intrackCat->selectedItemCode == NULL) {
		printMessage("ERROR: Select some items to remove from a category", intrackCat->mainWindow);
		return 1;
	}
	
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(intrackCat->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Remove items?");
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Remove Items");
	widgetDialog = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (widgetDialog) {
		case GTK_RESPONSE_YES:
			prepareItemRemove(NULL, intrackCat);
			break;
		case GTK_RESPONSE_NO:
			break;
	}

	return 0;
}

/* Destroy a GtkWidget */
static void destroyWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_destroy(GTK_WIDGET(data));
}


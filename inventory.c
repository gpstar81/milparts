//      inventory.c
//      Copyright 2010 - 2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For inventory control, mass listing, searching and editing via a gtktree.

// Use lock tables to prevent multiple sessions from working on the database, and transactions to keep data reliable.
// http://www.mysqltutorial.org/mysql-transaction.aspx

#include <mysql.h>
#include <stdio.h>
#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <time.h>
#include <gdk/gdkkeysyms.h>
#include <locale.h>
#include "settings.h"
#include "messages.h"
#include "inventory.h"

int column_counter = 0;
gchar *yearTitle1, *yearTitle2, *yearTitle3, *yearTitle4;

/*
TODO: Build serial # recording.
-----------------
|				|
|  Edit Serials |	<------- Button on toolbar to edit and list the serial numbers for the selected item in stock.
|				|
-----------------

(DO THIS STUFF AFTER)		
***** For this, going to have to put another hidden column, which records if serial numbers are missing or not. *****
Serial Number Code current status:
	-Creation and deletion of serial number database and tables. Table names are based on the product barcode. (COMPLETE).
		-When user adjusts the stock quantity with items that have serial numbers, a popup window comes up to choose which serial number items to remove. 
			(Uses check marks to select a column)
			-Multiple columns if the item uses multiple serial numbers per item.
		-When user adjusts the stock quantity with items that have serial numbers, a popup window comes up with a editable list store to enter serial numbers.
			(Uses check marks to select a column)
			-Multiple columns if the item uses multiple serial numbers per item.
	-Then modify checkout.c and checkout_submit.c to accept the recorded serial numbers via search pulldown or scanning.
		-EXTRA NOTE: Use scan override to bypass "Serial Number not found in stock" when a item serial number was never recorded into stock.

TODO: Things to add to items:
	-Last ordered -> This can be put in orders.c
	-Next Arrival -> This can be put in orders.c
	-On Order -> This can be put in orders.c
	-Weight
	-Location
	-Inventory Value
TODO: checkIfExist() needs to be fixed. Look at accounts.c checkAccountExist() for proper implementation.

#### Color Coding Table ####
Red = No Stock.
Yellow = Stock is below stock low level indicator.
Purple = Serial numbers missing.
*/

void initalizeInventory(GtkBuilder *builder, gchar *mysqlDatabase, gchar *inventoryTable, GtkWidget *mainWindow) {
	
	setlocale(LC_ALL, "");
	
	// Initialize the inventory structure
	intrackInventory *inventory;
	inventory = (intrackInventory*) g_malloc (sizeof (intrackInventory)); // Allocate memory for the data.
	
	inventory->mysqlDatabase = g_strconcat(mysqlDatabase, NULL);
	inventory->inventoryTable = g_strconcat(inventoryTable, NULL);
	inventory->mainWindow = mainWindow;
	inventory->selectedItemCode	 = g_strconcat(NULL, NULL);
	inventory->exportQueryString = g_strconcat(NULL, NULL);
	
	initalizeHideUnhideCounters(inventory);
	
    // Exit program button from file / quit.
    GtkWidget	*exitInventory;
    exitInventory = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryCloseButton"));
   	g_signal_connect(G_OBJECT(exitInventory), "activate", G_CALLBACK(hideGtkWidget), mainWindow);		
   	
   	/* Toolbar buttons */
   	GtkWidget	*addButton;
   	addButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenAddButton"));
   	g_signal_connect(G_OBJECT(addButton), "clicked", G_CALLBACK(prepareAddItem), inventory);
   	
   	GtkWidget	*categoryButton;
   	categoryButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenCategoryButton"));
   	g_signal_connect(G_OBJECT(categoryButton), "clicked", G_CALLBACK(openCategories), mainWindow);
   	
   	inventory->removeItemButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenDelButton"));
   	g_signal_connect(G_OBJECT(inventory->removeItemButton), "clicked", G_CALLBACK(deleteItemWindow), inventory);
   	gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
   	
   	/*
   	inventory->invenViewItemButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenViewItemButton"));
   	g_signal_connect(G_OBJECT(inventory->invenViewItemButton), "clicked", G_CALLBACK(prepareViewWindow), inventory);   	
   	gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
   	
   	inventory->editItemButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenEditButton"));
   	g_signal_connect(G_OBJECT(inventory->editItemButton), "clicked", G_CALLBACK(prepareEditWindow), inventory);
   	gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
   	*/
   	GtkWidget	*exportButton;
   	exportButton = GTK_WIDGET(gtk_builder_get_object(builder, "exportInventoryButton"));
   	g_signal_connect(G_OBJECT(exportButton), "clicked", G_CALLBACK(prepareExport), inventory);
   	
   	GtkWidget	*importButton;
   	importButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenImportButton"));
   	g_signal_connect(G_OBJECT(importButton), "clicked", G_CALLBACK(prepareImport), inventory);

   	// Information total labels
   	inventory->numberOfPartsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "numberOfPartsLabel"));
   	inventory->partCostsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsLabel"));
   	inventory->partCostsAvgLabel = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsAvgLabel"));
	inventory->ytdItemsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "ytdItems"));
	inventory->ytdSalesLabel = GTK_WIDGET(gtk_builder_get_object(builder, "ytdSales"));
	inventory->ytdEstSalesLabel = GTK_WIDGET(gtk_builder_get_object(builder, "ytdEstSales"));
	
    // Close the window
    GtkWidget	*inventoryCloseButtonBar;
    inventoryCloseButtonBar = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryCloseButtonBar"));
   	g_signal_connect(G_OBJECT(inventoryCloseButtonBar), "clicked", G_CALLBACK(hideGtkWidget), mainWindow);	   	
	
	// Setup the inventory tree
    struct tm *ptr;
    time_t lt;
    char str[80];
	char *text;
	
    lt = time(NULL);
    ptr = localtime(&lt);

    strftime(str, 100, "%G", ptr);
    
    // Global variable at top of this file. Used by column hide/unhide and column name.
    // These do not get freed from memory. Potential memory leak if I decide later to destroy the window instead of hiding it currently.
    yearTitle1 = g_strconcat(str, " YTD", NULL);
    yearTitle3 = g_strconcat(str, " YTD $", NULL);
    yearTitle2 = g_strconcat(str, " EST", NULL);
	yearTitle4 = g_strconcat(str, " EST $", NULL);
		
	inventory->inventoryViewport = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryViewport"));
	inventory->inventoryTree = gtk_tree_view_new();
	setupInventoryTree(inventory);
	gtk_container_add(GTK_CONTAINER(inventory->inventoryViewport), inventory->inventoryTree);
	
	inventory->inventorySearchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "inventorySearchEntry"));
	g_signal_connect(G_OBJECT(inventory->inventorySearchEntry), "activate", G_CALLBACK(getInventory), inventory);

	GtkWidget *inventorySearchButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventorySearchButton"));
	g_signal_connect(G_OBJECT(inventorySearchButton), "clicked", G_CALLBACK(getInventory), inventory);
	
	inventory->searchSpinMin = GTK_WIDGET(gtk_builder_get_object(builder, "searchSpinMin"));
	inventory->searchSpinMax = GTK_WIDGET(gtk_builder_get_object(builder, "searchSpinMax"));
	g_signal_connect(G_OBJECT(inventory->searchSpinMin), "activate", G_CALLBACK(getInventoryNumbers), inventory);
	g_signal_connect(G_OBJECT(inventory->searchSpinMax), "activate", G_CALLBACK(getInventoryNumbers), inventory);

	GtkWidget *inventoryNumberSearchButton = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryNumberSearchButton"));
	g_signal_connect(G_OBJECT(inventoryNumberSearchButton), "clicked", G_CALLBACK(getInventoryNumbers), inventory);
	
	// Now set the tree so it can handle multiple selections.
	//GtkTreeSelection *selection;
	inventory->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (inventory->inventoryTree));
	gtk_tree_selection_set_mode (inventory->selection, GTK_SELECTION_MULTIPLE);
	
	// Setup the inventory search checkbox buttons.
	inventory->barcodeSearch = GTK_WIDGET(gtk_builder_get_object(builder, "barcodeSearch"));
	inventory->descriptionSearch = GTK_WIDGET(gtk_builder_get_object(builder, "descriptionSearch"));
	inventory->categorySearch = GTK_WIDGET(gtk_builder_get_object(builder, "categorySearch"));
	inventory->replaceSearch = GTK_WIDGET(gtk_builder_get_object(builder, "replaceSearch"));
	inventory->costSearch = GTK_WIDGET(gtk_builder_get_object(builder, "costSearch")); /* Belongs to number search */
	inventory->priceSearch = GTK_WIDGET(gtk_builder_get_object(builder, "priceSearch")); /* Belongs to number search */
	inventory->stockSearch = GTK_WIDGET(gtk_builder_get_object(builder, "stockSearch")); /* Belongs to number search */
	
	inventory->hide_button = GTK_WIDGET(gtk_builder_get_object(builder, "hide_button"));
   	g_signal_connect(G_OBJECT(inventory->hide_button), "clicked", G_CALLBACK(prepareHideColumns), inventory);
	
	// Setup inventory column hide/unhide buttons.
	/*
	inventory->part_button = GTK_WIDGET(gtk_builder_get_object(builder, "part_button"));
   	g_signal_connect(G_OBJECT(inventory->part_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);
	
	inventory->desc_button = GTK_WIDGET(gtk_builder_get_object(builder, "desc_button"));
   	g_signal_connect(G_OBJECT(inventory->desc_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->extrainfo_button = GTK_WIDGET(gtk_builder_get_object(builder, "extrainfo_button"));
	g_signal_connect(G_OBJECT(inventory->extrainfo_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->manu_button = GTK_WIDGET(gtk_builder_get_object(builder, "manu_button"));
   	g_signal_connect(G_OBJECT(inventory->manu_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->replace_button = GTK_WIDGET(gtk_builder_get_object(builder, "replace_button"));
   	g_signal_connect(G_OBJECT(inventory->replace_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->weight_button = GTK_WIDGET(gtk_builder_get_object(builder, "weight_button"));
   	g_signal_connect(G_OBJECT(inventory->weight_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->disc_button = GTK_WIDGET(gtk_builder_get_object(builder, "disc_button"));
   	g_signal_connect(G_OBJECT(inventory->disc_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	
	inventory->cost_button = GTK_WIDGET(gtk_builder_get_object(builder, "cost_button"));
   	g_signal_connect(G_OBJECT(inventory->cost_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->costavg_button = GTK_WIDGET(gtk_builder_get_object(builder, "costavg_button"));
   	g_signal_connect(G_OBJECT(inventory->costavg_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->costavgttl_button = GTK_WIDGET(gtk_builder_get_object(builder, "costavgttl_button"));
   	g_signal_connect(G_OBJECT(inventory->costavgttl_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);
	
	inventory->profit_button = GTK_WIDGET(gtk_builder_get_object(builder, "profit_button"));
   	g_signal_connect(G_OBJECT(inventory->profit_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->price_button = GTK_WIDGET(gtk_builder_get_object(builder, "price_button"));
   	g_signal_connect(G_OBJECT(inventory->price_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->dealer_button = GTK_WIDGET(gtk_builder_get_object(builder, "dealer_button"));
   	g_signal_connect(G_OBJECT(inventory->dealer_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->stock_button = GTK_WIDGET(gtk_builder_get_object(builder, "stock_button"));
   	g_signal_connect(G_OBJECT(inventory->stock_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->order_button = GTK_WIDGET(gtk_builder_get_object(builder, "order_button"));
   	g_signal_connect(G_OBJECT(inventory->order_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->totalsold_button = GTK_WIDGET(gtk_builder_get_object(builder, "totalsold_button"));
   	g_signal_connect(G_OBJECT(inventory->totalsold_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->totalsoldamount_button = GTK_WIDGET(gtk_builder_get_object(builder, "totalsoldamount_button"));
   	g_signal_connect(G_OBJECT(inventory->totalsoldamount_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);

	inventory->lastsold_button = GTK_WIDGET(gtk_builder_get_object(builder, "lastsold_button"));
   	g_signal_connect(G_OBJECT(inventory->lastsold_button), "toggled", G_CALLBACK(ColumnButtonHideUnhide), inventory);
	*/
	
	// Setup popup menu when right clicking on the inventory tree.
	inventory->invenMenu = gtk_menu_new();
	
	
	inventory->invenMenuView = gtk_menu_item_new_with_label("View item");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuView);
	gtk_widget_show(inventory->invenMenuView);
	g_signal_connect(G_OBJECT(inventory->invenMenuView), "activate", G_CALLBACK(prepareViewWindow), inventory);
	
	/*
	inventory->invenMenuLink = gtk_menu_item_new_with_label("Create Label");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuLink);
	gtk_widget_show(inventory->invenMenuLink);
	g_signal_connect(G_OBJECT(inventory->invenMenuLink), "activate", G_CALLBACK(prepareUriLink), inventory);
	*/
	
	//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
	/*
	inventory->invenMenuEdit = gtk_menu_item_new_with_label("Edit item");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuEdit);
	gtk_widget_show(inventory->invenMenuEdit);
	g_signal_connect(G_OBJECT(inventory->invenMenuEdit), "activate", G_CALLBACK(prepareEditWindow), inventory);
	gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);	
	*/
	inventory->invenMenuRemove = gtk_menu_item_new_with_label("Remove item");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuRemove);
	gtk_widget_show(inventory->invenMenuRemove);
	g_signal_connect(G_OBJECT(inventory->invenMenuRemove), "activate", G_CALLBACK(deleteItemWindow), inventory);
	
	// Setup keypress signals on the tree
	g_signal_connect(inventory->inventoryTree, "button-press-event", G_CALLBACK(treeButtonPress), inventory);
	g_signal_connect(inventory->inventoryTree, "key-press-event", G_CALLBACK(treeKeyPress), inventory);
	
	// Load the inventory tree with all data from the database
	//getInventory(NULL, inventory);
	g_signal_connect(inventory->mainWindow, "show", G_CALLBACK(getInventory), inventory);
	
}

// Import a raw .csv file to the database. -> import.c
static void prepareImport(GtkWidget *widget, intrackInventory *inventory) {
	importDatabase(inventory->mainWindow); /* import.c */
}

// Export the inventory database. -> export.c
static void prepareExport(GtkWidget *widget, intrackInventory *inventory) {
	mysqlExportDatabase(NULL, inventory->mainWindow, inventory->exportQueryString); /* export.c */
}

// Opens the serial number editor. -> inventory_serial.c
static void prepareSerialWindow(GtkWidget *widget, intrackInventory *inventory) {
	//initalizeSerialEditor(inventory->mainWindow, inventory->selectedItemCode);
}

// Opens the category editor window. -> inventory_cat.c
static void openCategories(GtkWidget *widget, GtkWidget *mainWindow) {
	initalizeCategories(mainWindow); // -> inventory_cat.c
}

// Open up a window which allows full editing of a item. -> inventory_edit.c
static void prepareEditWindow(GtkWidget *widget, intrackInventory *inventory) {
	//initalizeEditor(inventory->mainWindow, inventory->inventoryTree, inventory->selectedItemCode, &inventory->serial, inventory->invenMenuSerialButton, inventory->invenSerialButton); // -> inventory_edit.c
}

// Open up a window which shows the item information (picture, info, stats, etc). -> view_item.c
static void prepareViewWindow(GtkWidget *widget, intrackInventory *inventory) {
	loadViewItem(inventory->mainWindow, inventory->selectedItemCode); // -> view_item.c
}

// Open up a a external uri link in the default os web browser.
static void prepareUriLink(GtkWidget *widget, intrackInventory *inventory) {
	g_timeout_add (1, urlOpen, (gpointer) inventory);
}

				 		
static gboolean urlOpen(gpointer data) {
	GError     *error2;
	gchar *uri;
	
	uri = g_strdup("http://barcode.millercanada.com");
	g_print("%s", uri);
	gtk_show_uri (NULL, uri, gtk_get_current_event_time(), &error2);
	g_free(uri);

	return FALSE;
}

// Opens a widget popup to prompt user to add a item. -> inventory_add.c
static void prepareAddItem(GtkWidget *widget, intrackInventory *inventory) {
	initalizeAdd(inventory->mainWindow, inventory); // -> inventory_add.c
}

// Opens a popup to prompt user to hide / unhide gtk tree columns. -> inventory_hidemenu.c
static void prepareHideColumns(GtkWidget *widget, intrackInventory *inventory) {
	initalizeHideMenu(inventory->mainWindow, inventory); // -> inventory_hidemenu.c
}

// Setup initalization values for the hide / unhide of column flag identifiers
static void initalizeHideUnhideCounters(intrackInventory *inventory) {
	inventory->int_part_button = 1;
	inventory->int_desc_button = 1;
	inventory->int_extra_button = 1;
	inventory->int_manu_button = 1;
	inventory->int_replace_button = 1;
	inventory->int_weight_button = 1;
	inventory->int_disc_button = 1;
	inventory->int_cost_button = 1;
	inventory->int_costavgttl_button = 1;
	inventory->int_profit_button = 1;
	inventory->int_price_button = 1;
	inventory->int_dealer_button = 1;
	inventory->int_order_button = 1;
	inventory->int_stock_button = 1;
	inventory->int_costavg_button = 1;
	inventory->int_totalsold_button = 1;
	inventory->int_totalsoldamount_button = 1;
	inventory->int_lastsold_button = 1;
	
	inventory->int_category_button = 1;
	inventory->int_length_button = 1;
	inventory->int_width_button = 1;
	inventory->int_height_button = 1;
	inventory->int_margin_button = 1;
	inventory->int_ytd_button = 1;
	inventory->int_est_button = 1;
	inventory->int_ytd_amount_button = 1;
	inventory->int_est_amount_button = 1;
}

// A widget window to ask for confirmation upon deletion
static void deleteItemWindow(GtkWidget *widget, intrackInventory *inventory) {
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(inventory->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Remove selected items ?");
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Remove Items?");
	widgetDialog = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (widgetDialog) {
		case GTK_RESPONSE_YES:
			prepareItemRemoval(NULL, inventory);
			break;
		
		case GTK_RESPONSE_NO:
			break;
	}
}

// Pulls inventory data when searching by numbers ie: cost, price and stock
static void getInventoryNumbers(GtkWidget *widget, intrackInventory *inventory) {
	/* Clear out selected item before refreshing the tree */
	g_free(inventory->selectedItemCode);
	inventory->selectedItemCode = g_strconcat(NULL, NULL);
   	gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
    //gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
   	//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);

	GtkTreeStore *store;
	
	//store = gtk_tree_store_new(INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT);
	store = gtk_tree_store_new(INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT);

	float min, max;
	
	min = gtk_spin_button_get_value(GTK_SPIN_BUTTON(inventory->searchSpinMin));
	max = gtk_spin_button_get_value(GTK_SPIN_BUTTON(inventory->searchSpinMax));
	
	if(max >= min) {
		gchar *minChar, *maxChar;
		minChar = g_strdup_printf("%f", min);
		maxChar = g_strdup_printf("%f", max);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->costSearch)))
			pullInventory(inventory, store, inventory->inventoryTable, "a.cost", NULL, minChar, maxChar);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->priceSearch)))
			pullInventory(inventory, store, inventory->inventoryTable, "a.price", NULL, minChar, maxChar);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->stockSearch)))
			pullInventory(inventory, store, inventory->inventoryTable, "a.stock", NULL, minChar, maxChar);
			
		g_free(minChar);
		g_free(maxChar);
	}
	else
		printMessage(ERROR_SEARCH_NUMBER, inventory->mainWindow);

	gtk_tree_view_set_model(GTK_TREE_VIEW(inventory->inventoryTree), GTK_TREE_MODEL(store));
		
	calculateTreeTotals(store, inventory);
	
	g_object_unref(store);
}

// Prepare to pull inventory data from the database
static void getInventory(GtkWidget *widget, intrackInventory *inventory) {
	// Clear out selected item before refreshing the tree
	g_free(inventory->selectedItemCode);
	inventory->selectedItemCode = g_strconcat(NULL, NULL);
   	gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
   	//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
   	//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
	
	GtkTreeStore *store;
	
	store = gtk_tree_store_new(INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_STRING, G_TYPE_FLOAT);
	
	// Checks to see if a ' is entered, that causes mysql.h to break
	if(g_strrstr(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry)), "'")) {
		printMessage("ERROR: ' not allowed.", inventory->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(inventory->inventorySearchEntry), ""); // Clear out the search entry
	}
	// This will search the inventory for the user query if the search entry is greater than 2 characters
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry))) > 2) {
		gchar *searchString;
		gchar *searchTemp;
		gchar *searchFinal;
		int counter = 0;
		searchString = g_strconcat(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry)), NULL);
		
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->barcodeSearch))) {
			searchFinal = g_strconcat("partNo LIKE '%", searchString, "%'", NULL);
			//pullInventory(inventory, store, inventory->inventoryTable, "a.partNo", searchString, NULL, NULL);
			counter++;
		}
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->descriptionSearch))) {
			if(counter > 0) {
				searchTemp = g_strconcat(searchFinal, " OR description LIKE '%", searchString, "%'", NULL);
				g_free(searchFinal);
				searchFinal = g_strdup(searchTemp);
				g_free(searchTemp);
			}
			else
				searchFinal = g_strconcat("description LIKE '%", searchString, "%'", NULL);
			//pullInventory(inventory, store, inventory->inventoryTable, "a.description", searchString, NULL, NULL);
			
			counter++;
		}
			
		// Category search. display_name field is name of the category in category tables. referenced by ids.
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->categorySearch))) {
			if(counter > 0) {
				searchTemp = g_strconcat(searchFinal, " OR display_name LIKE '%", searchString, "%'", NULL);
				g_free(searchFinal);
				searchFinal = g_strdup(searchTemp);
				g_free(searchTemp);
			}
			else
				searchFinal = g_strconcat("display_name LIKE '%", searchString, "%'", NULL);
			//pullInventory(inventory, store, inventory->inventoryTable, "b.category", searchString, NULL, NULL);	
			
			counter++;		
		}
			
		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->replaceSearch))) {
			if(counter > 0) {
				searchTemp = g_strconcat(searchFinal, " OR replacePart LIKE '%", searchString, "%'", NULL);
				g_free(searchFinal);
				searchFinal = g_strdup(searchTemp);
				g_free(searchTemp);
			}
			else			
				searchFinal = g_strconcat("replacePart LIKE '%", searchString, "%'", NULL);
			//pullInventory(inventory, store, inventory->inventoryTable, "a.replacePart", searchString, NULL, NULL);
			
			counter++;
		}
		
		pullInventory(inventory, store, inventory->inventoryTable, searchFinal, searchString, NULL, NULL);
		
		g_free(searchFinal);
		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(inventory->inventorySearchEntry), "");
	}
	// If the search entry is less than 3 characters but greater than 0
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, inventory->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(inventory->inventorySearchEntry), "");
	}
	else
		pullInventory(inventory, store, inventory->inventoryTable, NULL, NULL, NULL, NULL); // This pulls all the inventory out of the database
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(inventory->inventoryTree), GTK_TREE_MODEL(store));
	
	calculateTreeTotals(store, inventory);
	
	g_object_unref(store);
}

// Pulls the inventory data out of the database and stores it into a GtkListStore
static int pullInventory(intrackInventory *inventory, GtkTreeStore *store, gchar *inventoryTable, gchar *searchWhere, gchar *searchString, gchar *searchNumMin, gchar *searchNumMax) {
	GtkTreeIter 	iter;
	
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;	
	
	MYSQL *partsConnection, partsMysql;
	MYSQL_RES *partsResult;
	MYSQL_ROW partsRow;
	
	mysql_init(&partsMysql);
	
	// Open connection to the database
	partsConnection = mysql_real_connect(&partsMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);
	
	if(partsConnection == NULL) {
		printf(mysql_error(partsConnection), "%d\n");

		printMessage("CONNECTION FAILED", NULL);
		mysql_close(partsConnection);
		return 1;
	}	
	
    // Select the database.
    query_string = g_strconcat(mysqlDatabase, NULL);	
	query_state = mysql_select_db(partsConnection, query_string);	
	
	// Failed to connect and select database.
	if (query_state != 0) {
		printf(mysql_error(partsConnection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(partsConnection);
		
		return 1;
	}

	g_free(query_string);

	// Search for the user entered query
	if(searchString != NULL)
		query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.replacePart, a.cost, a.costAvg, a.price, a.dealer, a.stock, a.lowStockLvl, a.lastSold, a.discontinued, a.totalSold, a.manufacturer, a.weight, a.yearEst, a.yearTotalSold, a.salesTotalAmount, a.yearTotalAmount, a.shiplength, a.shipheight, a.shipwidth, b.display_name, a.extraInfo FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id", " WHERE ", searchWhere, NULL);
		//query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.replacePart, a.cost, a.costAvg, a.price, a.dealer, a.stock, a.lowStockLvl, a.lastSold, a.discontinued, a.totalSold, a.manufacturer, a.weight, a.yearEst, a.yearTotalSold, a.salesTotalAmount, a.yearTotalAmount, a.shiplength, a.shipheight, a.shipwidth, b.display_name, a.extraInfo FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id", " WHERE ", searchWhere, " LIKE '%", searchString, "%'", NULL);
	else if(searchNumMin != NULL && searchNumMax != NULL)
		query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.replacePart, a.cost, a.costAvg, a.price, a.dealer, a.stock, a.lowStockLvl, a.lastSold, a.discontinued, a.totalSold, a.manufacturer, a.weight, a.yearEst, a.yearTotalSold, a.salesTotalAmount, a.yearTotalAmount, a.shiplength, a.shipheight, a.shipwidth, b.display_name, a.extraInfo FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id", " WHERE ", searchWhere, " >= ", searchNumMin, " AND ", searchWhere, " <= ", searchNumMax, NULL);
	else
		query_string = g_strconcat("SELECT a.id, a.partNo, a.description, a.replacePart, a.cost, a.costAvg, a.price, a.dealer, a.stock, a.lowStockLvl, a.lastSold, a.discontinued, a.totalSold, a.manufacturer, a.weight, a.yearEst, a.yearTotalSold, a.salesTotalAmount, a.yearTotalAmount, a.shiplength, a.shipheight, a.shipwidth, b.display_name, a.extraInfo FROM `", INVENTORY_TABLES, "` a left join miller_website_config.", CATEGORY_TABLES ," b on a.category = b.id ", NULL);
		//query_string = g_strconcat("SELECT id, partNo, description, replacePart, cost, costAvg, price, dealer, stock, lowStockLvl, lastSold, discontinued, totalSold, manufacturer, weight, yearEst, yearTotalSold, salesTotalAmount, yearTotalAmount, shiplength, shipheight, shipwidth, category, extraInfo FROM `", INVENTORY_TABLES, "`", NULL);
	
	//g_print("%s\n", query_string);
	//Keep a copy of the current query for export purposes.
	g_free(inventory->exportQueryString);
	inventory->exportQueryString = g_strconcat(query_string, NULL);

	// If the connection is successful and the query returns a result then the next step is to display those results:
	mysql_query(partsConnection, query_string);
	partsResult = mysql_store_result(partsConnection);
	num_fields = mysql_num_fields(partsResult);

	while ((partsRow = mysql_fetch_row(partsResult))) {
		gchar *id, *barcode, *description, *replace, *cost, *costAverage, *costAverageTTL, *price, *stock, *lowStockLvl, *lastSold, *profitChar, *totalSoldChar, *manufacturer, *weight, *category, *extraInfo;
		
		gchar *shiplength, *shipwidth, *shipheight;
		int shiplengthi = 0;
		int shipwidthi = 0;
		int shipheighti = 0;
		
		gboolean discontinued = FALSE;
		gboolean dealer = FALSE;
		
		int yearEst = 0;
		int yearTotalSold = 0;
		float yearTotalSoldF = 0.00;
		gchar *yearEstChar, *yearTotalSoldChar, *yearTotalSoldAmountChar;
		gchar *salesTotalAmountChar;
		
		int totalSold = 0;
		int lowStockInt = 0;
		int stockInt = 0;
		float priceFloat = 0.00;
		float costFloat = 0.00;
		float costAverageFloat = 0.00;
		float costAverageTTLFloat = 0.00;
		float profitFloat = 0.00;
		float weightFloat = 0.00;
		float salesTotalAmount = 0.00;
		
		for(i = 0; i <= num_fields; i++) {
			if(i == 0)
				id = g_strdup(partsRow[i]);

			if(i == 1)
				barcode = g_strconcat(partsRow[i], NULL);
			
			if(i == 2)
				description = g_strconcat(partsRow[i], NULL);
				
			if(i == 3)
				replace = g_strconcat(partsRow[i], NULL);
				
			if(i == 4) {
				cost = g_strdup_printf("%.2f", atof(partsRow[i]));
				costFloat = atof(partsRow[i]);
				//cost = g_strconcat(partsRow[i], NULL);
			}
			
			if(i == 5) {
				costAverage = g_strdup_printf("%.2f", atof(partsRow[i]));
				costAverageFloat = atof(partsRow[i]);
				//costAverage = g_strdup_printf("%.2f", getAverageCost(barcode)); //costAverage = g_strconcat(row[i], NULL);
			}
			
			if(i == 6) {
				price = g_strdup_printf("%.2f", atof(partsRow[i]));
				priceFloat = atof(partsRow[i]);
				//price = g_strconcat(partsRow[i], NULL);
			}
			/*
			if(i == 7)
				dealer = g_strdup_printf("%.2f", atof(partsRow[i]));				
			*/
			
			if(i == 7) {
				if(atoi(partsRow[i]) == 1)
					dealer = TRUE;
				else
					dealer = FALSE;
			}
						
			if(i == 8) {
				stock = g_strconcat(partsRow[i], NULL);
				stockInt = atoi(partsRow[i]);
			}
			
			if(i == 9) {
				lowStockLvl = g_strdup(partsRow[i]);
				lowStockInt = atoi(partsRow[i]);
			}
				
			if(i == 10)
				lastSold = g_strconcat(partsRow[i], NULL);
				
			if(i == 11) {
				if(atoi(partsRow[i]) == 1)
					discontinued = TRUE;
				else
					discontinued = FALSE;
			}
			
			if(i == 12) {
				totalSoldChar = g_strdup(partsRow[i]);
				totalSold = atoi(partsRow[i]);
			}
			
			if(i == 13) {
				manufacturer = g_strdup(partsRow[i]);
			}		
			
			if(i == 14) {
				weight = g_strdup_printf("%.3f", atof(partsRow[i]));
				weightFloat = atof(partsRow[i]);
				//cost = g_strconcat(partsRow[i], NULL);
			}			

			if(i == 15) {
				yearEstChar = g_strdup(partsRow[i]);
				yearEst = atoi(partsRow[i]);
			}	
			
			if(i == 16) {
				yearTotalSoldChar = g_strdup(partsRow[i]);
				yearTotalSold = atoi(partsRow[i]);
			}		
			
			if(i == 17) {
				salesTotalAmountChar = g_strdup_printf("%'0.2f", atof(partsRow[i]));
				salesTotalAmount = atof(partsRow[i]);
			}		
			
			if(i == 18) {
				yearTotalSoldAmountChar = g_strdup_printf("%'0.2f", atof(partsRow[i]));
				yearTotalSoldF = atof(partsRow[i]);
			}
		
			if(i == 19) {
				shiplength = g_strdup_printf("%i", atoi(partsRow[i]));
				shiplengthi = atoi(partsRow[i]);				
			}

			if(i == 20) {
				shipheight = g_strdup_printf("%i", atoi(partsRow[i]));
				shipheighti = atoi(partsRow[i]);				
			}
			
			if(i == 21) {
				shipwidth = g_strdup_printf("%i", atoi(partsRow[i]));
				shipwidthi = atoi(partsRow[i]);			
			}					
			
			if(i == 22) {
				category = g_strdup(partsRow[i]);
			}
			
			if(i == 23) {
				extraInfo = g_strdup(partsRow[i]);
			}									
				//g_print("%i\n", i);
		}
		
		profitChar = g_strdup_printf("%.2f", atof(price) - atof(costAverage));
		profitFloat = priceFloat - costAverageFloat;
		
		costAverageTTL = g_strdup_printf("%.2f", atof(stock) * atof(costAverage));
		costAverageTTLFloat = stockInt * atof(costAverage);
		
		gchar *marginChar;
		float marginFloat;
		marginFloat = ((priceFloat - costFloat) / priceFloat) * 100;
		marginChar = g_strdup_printf("%.2f%%", marginFloat);		
		
		float yearEstAmountF = 0.00;
		gchar *yearEstAmountChar;
		
		yearEstAmountF = yearEst * priceFloat;
		yearEstAmountChar = g_strdup_printf("%'0.2f", yearEstAmountF);
		/*
		gchar *totalSoldChar;
		
		if(g_strrstr(lastSold,"0000-00-00 00:00:00")) {
			totalSoldChar = g_strdup_printf("%i", 0);
		} 
		else {
			int totalSoldInt = getCountSold(barcode);
		
			if(totalSoldInt > 0)
				totalSoldChar = g_strdup_printf("%i", totalSoldInt);
			else
				totalSoldChar = g_strdup_printf("%i", 0);
		}
		*/
		
		// Now search the tree store to see if the item already exists in it, we search via the item code for identification. This prevents duplicates from getting displayed
      	if(searchString != NULL || (searchNumMin != NULL && searchNumMax != NULL)) {
			gchar *rowtext;
			gboolean foundItem = FALSE;
			
			if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
				gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, ID, &rowtext, -1); // the id column
		
				if(rowtext != NULL) {
					// Found item
					if(!strcmp(rowtext, id))
						foundItem = TRUE;
						
					g_free(rowtext);
				}
				
				// Finish off searching the rest of the rows in the store
				while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)) {			
					gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, ID, &rowtext, -1); // the id column

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
				gtk_tree_store_append (store, &iter, NULL);
				
				gtk_tree_store_set (store, &iter, ID, id, BARCODE, barcode, DESCRIPTION, description, EXTRAINFO, extraInfo, MANUFACTURER, manufacturer, CATEGORY, category, REPLACE, replace, WEIGHT, weight, LENGTH, shiplength, WIDTH, shipwidth, HEIGHT, shipheight, DISCONTINUED, discontinued, COST, cost, COSTAVG, costAverage, COSTAVGTTL, costAverageTTL, PROFIT, profitChar, MARGIN, marginChar, PRICE, price, DEALER, dealer, STOCK, stock, LOWSTOCKLVL, lowStockLvl, TOTAL_SOLD, totalSold, TOTAL_SOLD_AMOUNT, salesTotalAmountChar, TOTAL_SOLD_AMOUNTF, salesTotalAmount, LAST_SOLD, lastSold, COSTF, costFloat, COSTAVGF, costAverageFloat, COSTAVGTTLF, costAverageTTLFloat, PROFITF, profitFloat, MARGINF, marginFloat, PRICEF, priceFloat, WEIGHTF, weightFloat, LENGTHI, shiplengthi, WIDTHI, shipwidthi, HEIGHTI, shipheighti, STOCKF, stockInt, LOWSTOCKLVLF, lowStockInt, YEARTOTAL, yearTotalSoldChar, YEARTOTALINT, yearTotalSold, YEAREST, yearEstChar, YEARESTINT, yearEst, YEARTOTALAMOUNT, yearTotalSoldAmountChar, YEARTOTALAMOUNTF, yearTotalSoldF, YEARESTAMOUNT, yearEstAmountChar, YEARESTAMOUNTF, yearEstAmountF, -1);	
			}			
		}
		else {
			// This stores all inventory into the tree
			gtk_tree_store_append (store, &iter, NULL);

			gtk_tree_store_set (store, &iter, ID, id, BARCODE, barcode, DESCRIPTION, description, EXTRAINFO, extraInfo, MANUFACTURER, manufacturer, CATEGORY, category, REPLACE, replace, WEIGHT, weight, LENGTH, shiplength, WIDTH, shipwidth, HEIGHT, shipheight, DISCONTINUED, discontinued, COST, cost, COSTAVG, costAverage, COSTAVGTTL, costAverageTTL, PROFIT, profitChar, MARGIN, marginChar, PRICE, price, DEALER, dealer, STOCK, stock, LOWSTOCKLVL, lowStockLvl, TOTAL_SOLD, totalSold, TOTAL_SOLD_AMOUNT, salesTotalAmountChar, TOTAL_SOLD_AMOUNTF, salesTotalAmount, LAST_SOLD, lastSold, COSTF, costFloat, COSTAVGF, costAverageFloat, COSTAVGTTLF, costAverageTTLFloat, PROFITF, profitFloat, MARGINF, marginFloat, PRICEF, priceFloat, WEIGHTF, weightFloat, LENGTHI, shiplengthi, WIDTHI, shipwidthi, HEIGHTI, shipheighti, STOCKF, stockInt, LOWSTOCKLVLF, lowStockInt, YEARTOTAL, yearTotalSoldChar, YEARTOTALINT, yearTotalSold, YEAREST, yearEstChar, YEARESTINT, yearEst, YEARTOTALAMOUNT, yearTotalSoldAmountChar, YEARTOTALAMOUNTF, yearTotalSoldF, YEARESTAMOUNT, yearEstAmountChar, YEARESTAMOUNTF, yearEstAmountF, -1);	
		}
		
		g_free(id);
		g_free(barcode);
		g_free(description);
		g_free(extraInfo);
		g_free(category);
		g_free(replace);
		g_free(cost);
		g_free(costAverage);
		g_free(costAverageTTL);
		g_free(price);
		//g_free(dealer);
		g_free(stock);
		g_free(lowStockLvl);
		g_free(lastSold);
		g_free(profitChar);
		g_free(marginChar);
		g_free(totalSoldChar);
		g_free(manufacturer);
		g_free(weight);
		g_free(shiplength);
		g_free(shipwidth);
		g_free(shipheight);
		g_free(yearEstChar);
		g_free(yearTotalSoldChar);
		g_free(salesTotalAmountChar);
		g_free(yearTotalSoldAmountChar);
		g_free(yearEstAmountChar);
	}	

	g_free(query_string);	
	mysql_free_result(partsResult);
	mysql_close(partsConnection);

	return 0;
}

// Remove a item from the inventory database
static void beginItemRemoval(GtkTreeRowReference *ref, intrackInventory *inventory) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData, *serialChar;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &whereData, -1); // ID column

	// Update the database
	databaseRemoveItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData);
	
	// Write code here to update the barcode in taxGroupItems database
	//databaseTaxFeesItemsEdit(whereData, NULL, 1, "taxGroup", TAXES_TABLES, "TaxGroupsItems"); /* the 0 is the mode, in this case, 0 = edit the item. 1 = remove the item */
	//databaseTaxFeesItemsEdit(whereData, NULL, 1, "feeName", FEES_TABLES, "FeesItems");
	
	/* Remove the row from the tree */
	gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

	g_free(whereData);
}

// Prepares the remove process of a item from the inventory
static void prepareItemRemoval(GtkWidget *widget, intrackInventory *inventory) {

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;
	
   	gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
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
	g_list_foreach(references, (GFunc) beginItemRemoval, inventory);
	
	// Free the tree paths, tree row references and lists
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
}

// Prepares the remove process of a item from the inventory
// NOT USED ANY MORE, DELETE THIS FUNCTION
static void prepareItemRemovalOld(GtkWidget *widget, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    GtkTreePath 	*path;
    GtkTreeSelection *selection;

	gchar *whereData;
	
    // Need to pull the current selected row from the treeview
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));

	// Now need to pull the path and iter from the selected row
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);

		//model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
		//gtk_tree_model_get_iter_from_string(model, &iter, path);
		gtk_tree_model_get(model, &iter, BARCODE, &whereData, -1); // barcode column

		databaseRemoveItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData);

		gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);

		g_free(whereData);
	}
	else
		printMessage("ERROR: Select a item to remove from the inventory database", inventory->mainWindow);
}

// Removes a item from the inventory database
static int databaseRemoveItem(gchar *mysqlDatabase, gchar *inventoryTable, gchar *id) {

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
	query_string = g_strconcat("DELETE FROM `", inventoryTable, "`", " WHERE id='", id, "'", NULL);
	
	query_state=mysql_query(connection, query_string);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		g_free(query_string);	
		mysql_close(connection);

		return 1;
	}
	
	g_free(query_string);	
	mysql_close(connection);			
	
	return 0;
}

// Updates a item in the inventory database.
static int databaseQuery(gchar *database, gchar *query) {
	MYSQL *queryConnection, queryMysql;

	mysql_init(&queryMysql);
	queryConnection = mysql_real_connect(&queryMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);
		
	int query_state;	
	
	// Select the database.
	query_state = mysql_select_db(queryConnection, database);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(queryConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(queryConnection));
		printMessage(errorMessage, NULL);
		g_free(errorMessage);		
		
		mysql_close(queryConnection);
		
		return 1;
	}
	
	query_state=mysql_query(queryConnection, query);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(queryConnection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(queryConnection));
		printMessage(errorMessage, NULL);
		g_free(errorMessage);	
				
		mysql_close(queryConnection);

		return 1;
	}
	
	mysql_close(queryConnection);			
	return 0;	
}

// Updates a item in the inventory database.
static int databaseEditItem(gchar *mysqlDatabase, gchar *inventoryTable, gchar *barcode, gchar *newCode, gchar *column) {
	
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
	query_string = g_strconcat("UPDATE `", inventoryTable, "`", " SET ", column, "='", newCode, "' WHERE partNo='", barcode, "'", NULL);

	//g_print("%s\n", query_string);

	query_state=mysql_query(connection, query_string);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		g_free(query_string);	
		mysql_close(connection);

		return 1;
	}
	
	g_free(query_string);	
	mysql_close(connection);			
	
	return 0;	
}

// Updates a item in the inventory database.
int databaseEditItemByID(gchar *mysqlDatabase, gchar *inventoryTable, gchar *id, gchar *newCode, gchar *column) {
	
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
	query_string = g_strconcat("UPDATE `", inventoryTable, "`", " SET ", column, "='", newCode, "' WHERE id='", id, "'", NULL);

	query_state=mysql_query(connection, query_string);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		g_free(query_string);	
		mysql_close(connection);

		return 1;
	}
	
	g_free(query_string);	
	mysql_close(connection);			
	
	return 0;	
}

/*
int databaseTaxFeesItemsEdit(gchar *whereData, gchar *newText, int mode, gchar *selectData, gchar *tables, gchar *databaseName) {

	MYSQL *taxesConnection, taxesMysql;
	MYSQL_RES *taxesResult;
	MYSQL_ROW taxesRow;

	mysql_init(&taxesMysql);

	taxesConnection = mysql_real_connect(&taxesMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	// taxes uses a separate connection, if connection fails, check the network connection
	if (taxesConnection == NULL) {
		printf(mysql_error(taxesConnection), "%d\n");

		printMessage(ERROR_TAXES, NULL);
		mysql_close(taxesConnection);
		checkNetworkConnection();
		return 1;
	}

	gchar *query_string;
	int query_state;
	int num_fields;
	int i;		

	// Select the database.
	query_string = g_strconcat(mysqlDatabase, NULL);
	query_state = mysql_select_db(taxesConnection, query_string);
	
	g_free(query_string);
	query_string = g_strconcat("SELECT ", selectData, " FROM `", tables, "`", NULL);
	query_state = mysql_query(taxesConnection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(taxesConnection), "%d\n");
	}

	taxesResult = mysql_store_result(taxesConnection);
	num_fields = mysql_num_fields(taxesResult);
	
	// Run through all the taxes
	while ( ( taxesRow = mysql_fetch_row(taxesResult)) ) {
		for(i = 0; i < num_fields; i++) {
			// The Tax Name
			if(i == 0) {
				if(mode == 0)
					databaseTaxFeesItemsEdit2(whereData, newText, taxesRow[i], databaseName);
				
				if(mode == 1)
					databaseTaxFeesItemsRemove(whereData, taxesRow[i], databaseName);
			}
		}
	}

	g_free(query_string);

	mysql_free_result(taxesResult);
	mysql_close(taxesConnection);	
	
	return 0;
}
*/

/*
// Removes the item from each tax group
static int databaseTaxFeesItemsRemove(gchar *whereData, gchar *taxNameTable, gchar *databaseName) {
	
	MYSQL *taxesConnection, taxesMysql;

	mysql_init(&taxesMysql);

	taxesConnection = mysql_real_connect(&taxesMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	// taxes uses a separate connection, if connection fails, check the network connection
	if (taxesConnection == NULL) {
		printf(mysql_error(taxesConnection), "%d\n");

		printMessage(ERROR_TAXES, NULL);
		mysql_close(taxesConnection);
		checkNetworkConnection();
		return 1;
	}
	
	gchar *query_string;
	int query_state;

	// Select the database.
	query_string = g_strconcat(mysqlDatabase, databaseName, NULL);
	query_state = mysql_select_db(taxesConnection, query_string);	
	
	g_free(query_string);
	
	query_string = g_strconcat("DELETE FROM `", taxNameTable, "` WHERE barcode='", whereData, "'", NULL);
	
	query_state = mysql_query(taxesConnection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(taxesConnection), "%d\n");
	}
		
	g_free(query_string);

	mysql_close(taxesConnection);	
	
	return 0;
}
*/

// Makes changes to the tax group items
/*
static int databaseTaxFeesItemsEdit2(gchar *whereData, gchar *newText, gchar *taxNameTable, gchar *databaseName) {
	
	MYSQL *taxesConnection, taxesMysql;

	mysql_init(&taxesMysql);

	taxesConnection = mysql_real_connect(&taxesMysql,mysqlServer,mysqlUsername,mysqlPassword,MYSQL_DEFAULT_DATABASE, 0, 0, 0);

	// taxes uses a separate connection, if connection fails, check the network connection
	if (taxesConnection == NULL) {
		printf(mysql_error(taxesConnection), "%d\n");

		printMessage(ERROR_TAXES, NULL);
		mysql_close(taxesConnection);
		checkNetworkConnection();
		return 1;
	}
	
	gchar *query_string;
	int query_state;

	// Select the database.
	query_string = g_strconcat(mysqlDatabase, databaseName, NULL);
	query_state = mysql_select_db(taxesConnection, query_string);	
	
	g_free(query_string);
	
	query_string = g_strconcat("UPDATE `", taxNameTable, "` SET barcode='", newText, "' WHERE barcode='", whereData, "'" , NULL);
	query_state = mysql_query(taxesConnection, query_string);
	
	if (query_state !=0) {
		printf(mysql_error(taxesConnection), "%d\n");
	}
		
	g_free(query_string);

	mysql_close(taxesConnection);	
	
	return 0;
}
*/

// Edits the barcode
static int cellClickedBarcode(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	// Checks to see if input is valid
	//if(checkInput(newText))
		//return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); // id column	

    //if(strlen(newText) > 0 && checkIfExist(newText)) {
    if(strlen(newText) > 0) {
		//printMessage("Valid code. Now changing the code");
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "partNo");

		// Write code here to update the barcode in taxGroupItems database and fees
		//databaseTaxFeesItemsEdit(whereData, newText, 0, "taxGroup", TAXES_TABLES, "TaxGroupsItems"); /* the 0 is the mode, in this case, 0 = edit the item. 1 = remove the item */
		//databaseTaxFeesItemsEdit(whereData, newText, 0, "feeName", FEES_TABLES, "FeesItems");
		
		// Update the tree cell with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, BARCODE, newText, -1);
		
		//g_free(inventory->selectedItemCode);
		//inventory->selectedItemCode = g_strconcat(newText, NULL);
	}
	else if(strlen(newText) < 1) {
		printMessage(ERROR_LENGTH, inventory->mainWindow);
	}

    g_free(whereData);
	
	return 0;
}

// Edits the description
static int cellClickedDescription(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	// Checks to see if input is valid
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

    //if(!checkIfExist(whereData)) {
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "description");

		// Update the tree cell with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, DESCRIPTION, newText, -1);
	//}

    g_free(whereData);
	
	return 0;
}

// Edits the description
static int cellClickedExtraInfo(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* barcode column */	

    //if(!checkIfExist(whereData)) {
		
		/* Update the database */
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "extraInfo");

		/* Update the tree cell with the new data */
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, EXTRAINFO, newText, -1);
	//}

    g_free(whereData);
	
	return 0;
}

// Edits the manufacturer
static int cellClickedManufacturer(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	// Checks to see if input is valid
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;
	g_print("%s\n", path);

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);	

    //if(!checkIfExist(whereData)) {
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "manufacturer");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, MANUFACTURER, newText, -1);
	//}

    g_free(whereData);
	
	return 0;
}

static int cellClickedCategory(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;
    
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	/*
    GtkTreeIter iter;
	gchar *whereData;
	gtk_tree_selection_get_selected(selection, &model, &iter);
	gtk_tree_model_get(model, &iter, BARCODE, &whereData, -1);
	g_print("%s\n", whereData);
	*/
	//GtkTreePath	*new_path;
	
	ptr = rows;
	// Create tree row references to all of the selected rows
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		
		//new_path = gtk_tree_row_reference_get_path(ref);
		//gtk_tree_view_set_cursor (GTK_TREE_VIEW(inventory->inventoryTree), new_path, NULL, FALSE);
			
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	//gtk_tree_path_free(path);
	
	g_list_foreach(references, (GFunc) startEditCategory, inventory);

	// Free the tree paths, tree row references and lists
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
	
	// Quick category change via category_popup.c
    //categoryPopup(inventory->mainWindow, id_code, inventory->inventoryTree, TRUE); // -> category_popup.c
    
    
	/*
	// Checks to see if input is valid
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); // id column

    //if(!checkIfExist(whereData)) {
		
		// Update the database
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "category");

		// Update the tree cell with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, CATEGORY, newText, -1);
	//}

    g_free(whereData);
	*/
	return 0;
}

// Open the popup to edit the category for a item. called from -> cellClickedCategory()
static void startEditCategory(GtkTreeRowReference *ref, intrackInventory *inventory) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *row_id;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &row_id, -1);
    
    categoryPopup(inventory->mainWindow, row_id, inventory->inventoryTree, TRUE);
	
    g_free(row_id);
}

// Edits the replacement part number
static int cellClickedReplace(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

   // if(!checkIfExist(whereData)) {
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "replacePart");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, REPLACE, newText, -1);
	//}

    g_free(whereData);
	
	return 0;
}

// Sets discontinued status on a item.
static int cellClickedDiscontinued(GtkCellRendererToggle *render, gchar *path, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gboolean 		value;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);	
    gtk_tree_model_get(model, &iter, DISCONTINUED, &value, -1);

	gchar *newSerChar;
		
	if(value == FALSE)
		newSerChar = g_strdup_printf("%i", 1);
	else
		newSerChar = g_strdup_printf("%i", 0);
		
	databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newSerChar, "discontinued");

	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, DISCONTINUED, !value, -1);
		
	g_free(newSerChar);
    g_free(whereData);
	
	return 0;
}

// Quick category change via category_popup.c
//static void cellClickedCategory(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
    //categoryPopup(inventory->mainWindow, inventory->selectedItemCode, inventory->inventoryTree); // -> category_popup.c
//}

// Edits the cost
static int cellClickedCost(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	// Checks to see if input is valid
	//if(checkInput(newText))
		//return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *price;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);
   	gtk_tree_model_get(model, &iter, PRICE, &price, -1);    

   // if(!checkIfExist(whereData)) {
		gchar *newCostChar, *marginChar;
		float newCost, margin;

		newCost = atof(newText);
		newCostChar = g_strdup_printf("%.2f", newCost);
		
		// Update the database
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newCostChar, "cost");

		margin = ((atof(price) - newCost) / atof(price)) * 100;
		marginChar = g_strdup_printf("%.2f%%", margin);
		
		// Update the tree cell with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COST, newCostChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COSTF, newCost, -1);
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, MARGIN, marginChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, MARGINF, margin, -1);			

		g_free(newCostChar), g_free(marginChar);
		
		calculateTreeTotals(GTK_TREE_STORE(model), inventory);
	//}

    g_free(whereData), g_free(price);
	
	return 0;
}

// Edits the weight
static int cellClickedWeight(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

   // if(!checkIfExist(whereData)) {
		gchar *newWeightChar;
		float newWeight;

		newWeight = atof(newText);
		newWeightChar = g_strdup_printf("%.3f", newWeight);
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newWeightChar, "weight");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, WEIGHT, newWeightChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, WEIGHTF, newWeight, -1);

		g_free(newWeightChar);
		
		//calculateTreeTotals(GTK_TREE_STORE(model), inventory);
	//}

    g_free(whereData);
	
	return 0;
}

// Edits the length
static int cellClickedLength(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);	

		gchar *newDataChar;
		int newData;

		newData = atoi(newText);
		newDataChar = g_strdup_printf("%i", newData);
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newDataChar, "shiplength");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, LENGTH, newDataChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, LENGTHI, newData, -1);

		g_free(newDataChar);
		
    g_free(whereData);
	
	return 0;
}

// Edits the width
static int cellClickedWidth(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

		gchar *newDataChar;
		int newData;

		newData = atoi(newText);
		newDataChar = g_strdup_printf("%i", newData);
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newDataChar, "shipwidth");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, WIDTH, newDataChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, WIDTHI, newData, -1);

		g_free(newDataChar);
		
    g_free(whereData);
	
	return 0;
}

// Edits the height
static int cellClickedHeight(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

		gchar *newDataChar;
		int newData;

		newData = atoi(newText);
		newDataChar = g_strdup_printf("%i", newData);
		
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newDataChar, "shipheight");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, HEIGHT, newDataChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, HEIGHTI, newData, -1);

		g_free(newDataChar);
		
    g_free(whereData);
	
	return 0;
}

// Edits the price
static int cellClickedPrice(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

    //if(!checkIfExist(whereData)) {
		gchar *newPriceChar;
		gchar *costAvgChar, *profitChar, *marginChar;
		float costAvg, profit, margin;
		float newPrice;

		newPrice = atof(newText);
		newPriceChar = g_strdup_printf("%.2f", newPrice);
		
		// Update the database
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newPriceChar, "price");

		gtk_tree_model_get(model, &iter, COSTAVG, &costAvgChar, -1);
		
		profit = newPrice - atof(costAvgChar);
		profitChar = g_strdup_printf("%.2f", profit);

		margin = ((newPrice - atof(costAvgChar)) / newPrice) * 100;
		marginChar = g_strdup_printf("%.2f%%", margin);
		
		// Update the tree cell with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PRICE, newPriceChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PRICEF, newPrice, -1);
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PROFIT, profitChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PROFITF, profit, -1);
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, MARGIN, marginChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, MARGINF, margin, -1);			
		
		g_free(newPriceChar), g_free(costAvgChar), g_free(profitChar), g_free(marginChar);
		
		calculateTreeTotals(GTK_TREE_STORE(model), inventory);
	//}

    g_free(whereData);
	
	return 0;
}

// Edits the dealer price
/*
static int cellClickedDealer(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	// Checks to see if input is valid
	//if(checkInput(newText))
		//return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); // barcode column

    //if(!checkIfExist(whereData)) {
		gchar *newPriceChar;
		float newPrice;

		newPrice = atof(newText);
		newPriceChar = g_strdup_printf("%.2f", newPrice);
		
		// Update the database
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newPriceChar, "dealer");

		// Update the tree cell with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, DEALER, newPriceChar, -1);
		
		g_free(newPriceChar);
	//}

    g_free(whereData);
	
	return 0;
}
*/

// Sets discontinued status on a item.
static int cellClickedDealer(GtkCellRendererToggle *render, gchar *path, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gboolean 		value;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);	
    gtk_tree_model_get(model, &iter, DEALER, &value, -1);

	gchar *newSerChar;
		
	if(value == FALSE)
		newSerChar = g_strdup_printf("%i", 1);
	else
		newSerChar = g_strdup_printf("%i", 0);
		
	databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newSerChar, "dealer");

	gtk_tree_store_set(GTK_TREE_STORE(model), &iter, DEALER, !value, -1);
		
	g_free(newSerChar);
    g_free(whereData);
	
	return 0;
}

// Gets the average cost of the item
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
	costQuery = g_strconcat("SELECT costAvg FROM `", mysqlTables, "` WHERE id = '", barcode, "'", NULL);
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

// Get how many of a item has sold
static int getCountSold(gchar *barcode) {
	
	gchar *costQuery;
	int query_state, num_fields, i;
	int soldAmount;
		

	MYSQL *costConnection, costMysql;
	MYSQL_RES *costResult;
	MYSQL_ROW costRow;
	
	mysql_init(&costMysql);
	costConnection = mysql_real_connect(&costMysql, mysqlServer, mysqlUsername, mysqlPassword, MYSQL_DEFAULT_DATABASE, 0, 0, 0);
	
    costQuery = g_strconcat(mysqlDatabase, NULL);	
	query_state = mysql_select_db(costConnection, costQuery);		
	
	g_free(costQuery);
	costQuery = g_strconcat("SELECT count(*) FROM `", SOLD_TABLES, "` WHERE partNo = '", barcode, "'", NULL);
	query_state = mysql_query(costConnection, costQuery);

	costResult = mysql_store_result(costConnection);
	num_fields = mysql_num_fields(costResult);
	while ( ( costRow = mysql_fetch_row(costResult)) ) {
		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				soldAmount = atoi(costRow[i]);
		}
	}
	
	g_free(costQuery);
	
	mysql_free_result(costResult);
	mysql_close(costConnection);

	return soldAmount;
}

// Edit the stock
static int cellClickedStock(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar 		*whereData, *oldStockData, *oldAvgCostData, *costData;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);
    
    // Get the old stock, average cost and cost for recalculating the old average cost
    gtk_tree_model_get(model, &iter, STOCK, &oldStockData, -1);
    gtk_tree_model_get(model, &iter, COSTAVG, &oldAvgCostData, -1);
    gtk_tree_model_get(model, &iter, COST, &costData, -1);

    // if(!checkIfExist(whereData) && (strlen(newText) > 0) && (atoi(newText) != atoi(oldStockData))) {
    if((strlen(newText) > 0) && (atoi(newText) != atoi(oldStockData))) {
		gchar *newStockChar, *newAvgCostChar, *profitChar, *priceChar;
		int newStock, oldStock, stockDiff;
		float oldAvgCost, newAvgCost, cost;

		oldAvgCost = getAverageCost(whereData);

		newStock = atoi(newText);
		newStockChar = g_strdup_printf("%i", newStock);

		oldStock = atoi(oldStockData);
		//oldAvgCost = atof(oldAvgCostData);
		cost = atof(costData);
		
		// When stock is added
		if(newStock > oldStock) {
			stockDiff = newStock - oldStock;
			newAvgCost = ((oldAvgCost * oldStock) + (cost * stockDiff)) / (oldStock + stockDiff);
		}
		// When stock is subtracted
		else if((newStock < oldStock) && newStock > 0) {
			stockDiff = abs(oldStock - newStock);
			newAvgCost = ((oldAvgCost * oldStock) + ((oldAvgCost * stockDiff) * -1)) / (oldStock - stockDiff);
		}
		else { // When stock is now 0
			stockDiff = 0;
			newAvgCost = cost;
		}
		
		newAvgCostChar = g_strdup_printf("%f", newAvgCost);
				
		// Update the database
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newStockChar, "stock");
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newAvgCostChar, "costAvg"); // Update the new average cost

		g_free(newAvgCostChar);
		newAvgCostChar = g_strdup_printf("%0.2f", newAvgCost);
		
		gchar *newAvgCostTTL;
		newAvgCostTTL = g_strdup_printf("%0.2f", newAvgCost * newStock);

		gtk_tree_model_get(model, &iter, PRICE, &priceChar, -1);		
		float profit = atof(priceChar) - atof(newAvgCostChar);
		profitChar = g_strdup_printf("%.2f", profit);		

		// Update the tree cells with the new data
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, STOCK, newStockChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, STOCKF, newStock, -1);
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COSTAVG, newAvgCostChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COSTAVGF, newAvgCost, -1);
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COSTAVGTTL, newAvgCostTTL, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, COSTAVGTTLF, newAvgCost * newStock, -1);
		
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PROFIT, profitChar, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, PROFITF, profit, -1);
			
		
		g_free(newStockChar);
		g_free(newAvgCostChar), g_free(newAvgCostTTL);
		g_free(profitChar), g_free(priceChar);
		
		calculateTreeTotals(GTK_TREE_STORE(model), inventory);
	}

    g_free(whereData);
	g_free(oldStockData);
	g_free(oldAvgCostData);
	g_free(costData);
	
	return 0;
}

// Edits the low stock level indicator
static int cellClickedLowStockLVL(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *oldStockLvlData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1);

    // Get the old stock, average cost and cost for recalculating the old average cost
    gtk_tree_model_get(model, &iter, LOWSTOCKLVL, &oldStockLvlData, -1);
    
    //if(!checkIfExist(whereData) && (strlen(newText) > 0) && (atoi(newText) != atoi(oldStockLvlData)) && atoi(newText) >= 0) {
    if((strlen(newText) > 0) && (atoi(newText) != atoi(oldStockLvlData)) && atoi(newText) >= 0) {
		databaseEditItemByID(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "lowStockLvl");

		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, LOWSTOCKLVL, newText, -1);
		gtk_tree_store_set(GTK_TREE_STORE(model), &iter, LOWSTOCKLVLF, atoi(newText), -1);
	}

    g_free(whereData);
	g_free(oldStockLvlData);
	
	return 0;
}

static void ColumnButtonHideUnhide(GtkWidget *widget, intrackInventory *inventory) {
	// Force a redraw of the viewport to see updated changes to the gtk tree	
	gtk_widget_queue_draw(inventory->inventoryViewport);	
}

static void ColumnHideUnhide(GtkTreeViewColumn *column, intrackInventory *inventory) {
	if(g_strcmp0(gtk_tree_view_column_get_title(column), PART_COLUMN) == 0) {
		if(inventory->int_part_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);	
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), DESC_COLUMN) == 0) {
		if(inventory->int_desc_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);			
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), EXTRA_COLUMN) == 0) {
		if(inventory->int_extra_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);			
	}	
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), MANU_COLUMN) == 0) {
		if(inventory->int_manu_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), REPLACE_COLUMN) == 0) {
		if(inventory->int_replace_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), WEIGHT_COLUMN) == 0) {
		if(inventory->int_weight_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);		
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), DISC_COLUMN) == 0) {
		if(inventory->int_disc_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), COST_COLUMN) == 0) {
		if(inventory->int_cost_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), COSTAVG_COLUMN) == 0) {
		if(inventory->int_costavg_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);		
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), COSTAVGTTL_COLUMN) == 0) {
		if(inventory->int_costavgttl_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);	
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), PROFIT_COLUMN) == 0) {
		if(inventory->int_profit_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), PRICE_COLUMN) == 0) {
		if(inventory->int_price_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), DEALER_COLUMN) == 0) {
		if(inventory->int_dealer_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);	
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), STOCK_COLUMN) == 0) {
		if(inventory->int_stock_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}

	if(g_strcmp0(gtk_tree_view_column_get_title(column), ORDER_COLUMN) == 0) {
		if(inventory->int_order_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), TOTALSOLD_COLUMN) == 0) {
		if(inventory->int_totalsold_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);	
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), TOTALSOLDAMOUNT_COLUMN) == 0) {
		if(inventory->int_totalsoldamount_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);			
	}
		
	if(g_strcmp0(gtk_tree_view_column_get_title(column), LASTSOLD_COLUMN) == 0) {
		if(inventory->int_lastsold_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), CAT_COLUMN) == 0) {
		if(inventory->int_category_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), LENGTH_COLUMN) == 0) {
		if(inventory->int_length_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}	
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), WIDTH_COLUMN) == 0) {
		if(inventory->int_width_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), HEIGHT_COLUMN) == 0) {
		if(inventory->int_height_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), MARGIN_COLUMN) == 0) {
		if(inventory->int_margin_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), yearTitle1) == 0) {
		if(inventory->int_ytd_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), yearTitle2) == 0) {
		if(inventory->int_est_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), yearTitle3) == 0) {
		if(inventory->int_ytd_amount_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}
	
	if(g_strcmp0(gtk_tree_view_column_get_title(column), yearTitle4) == 0) {
		if(inventory->int_est_amount_button < 1)
			gtk_tree_view_column_set_visible(column, FALSE);
		else
			gtk_tree_view_column_set_visible(column, TRUE);
	}				
}

// Keep track of cell data based on state of the item.
static void cell_data_func(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
	gchar *colour, *stockChar, *lowStockLevelChar;
	int stock, lowStockLevel;
	gboolean	discontinued;

	intrackInventory *inventory = (intrackInventory *)data;

	// Determine if need to hide or unhide the column.
	ColumnHideUnhide(column, inventory);
	
	// Get the current stock level and adjust the cell colour accordingly.
	gtk_tree_model_get(model, iter, STOCK, &stockChar, -1);
	gtk_tree_model_get(model, iter, DISCONTINUED, &discontinued, -1);
	gtk_tree_model_get(model, iter, LOWSTOCKLVL, &lowStockLevelChar, -1);
	stock = atoi(stockChar);
	lowStockLevel = atoi(lowStockLevelChar);
	
	// Control and set cell renderer colour.
	if(stock < 1) {
		if(discontinued == TRUE) {
			colour = g_strdup("#7A7A7A");
		}
		else {
			colour = g_strdup("#FFDD85");
		}
	}
	else if(stock <= lowStockLevel) {
		if(discontinued == TRUE) {
			colour = g_strdup("#929292");
		}
		else {
			colour = g_strdup("#D58858");
		}
	}
	else {
		if(discontinued == TRUE) {
			colour = g_strdup("#BFBFBF");
		}
		else {
			colour = g_strdup("#FFFFFF");
		}
	}
	g_object_set(renderer, "cell-background", colour, "cell-background-set", TRUE, NULL);
  
	g_free(stockChar), g_free(lowStockLevelChar), g_free(colour);
}

// Create the tree view and columns
static void setupInventoryTree(intrackInventory *inventory) { 
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(PART_COLUMN, renderer, "text", BARCODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BARCODE); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedBarcode), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(DESC_COLUMN, renderer, "text", DESCRIPTION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DESCRIPTION); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDescription), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(EXTRA_COLUMN, renderer, "text", EXTRAINFO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, EXTRAINFO);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedExtraInfo), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);

	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(MANU_COLUMN, renderer, "text", MANUFACTURER, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, MANUFACTURER); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedManufacturer), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(CAT_COLUMN, renderer, "text", CATEGORY, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CATEGORY); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "editing-started", G_CALLBACK (cellClickedCategory), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);

	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(REPLACE_COLUMN, renderer, "text", REPLACE, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, REPLACE); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedReplace), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*weightAdj;
	weightAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.000, 0.000, 100000000.00, 0.001, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", weightAdj,
                            "digits", 3, NULL);
	column = gtk_tree_view_column_new_with_attributes(WEIGHT_COLUMN, renderer, "text", WEIGHT, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, WEIGHTF);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedWeight), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*lengthAdj;
	lengthAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 100000000.00, 1.0, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", lengthAdj,
                            "digits", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes(LENGTH_COLUMN, renderer, "text", LENGTH, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, LENGTHI);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedLength), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*widthAdj;
	widthAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 100000000.00, 1.0, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", widthAdj,
                            "digits", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes(WIDTH_COLUMN, renderer, "text", WIDTH, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, WIDTHI);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedWidth), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*heightAdj;
	heightAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 100000000.00, 1.0, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", heightAdj,
                            "digits", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes(HEIGHT_COLUMN, renderer, "text", HEIGHT, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, HEIGHTI);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedHeight), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes(DISC_COLUMN, renderer, "active", DISCONTINUED, NULL);
	gtk_tree_view_column_set_alignment(column, 0.5);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DISCONTINUED);
	g_object_set (renderer, "xalign", 0.5, NULL);
	g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (cellClickedDiscontinued), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*costAdj;
	costAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00, 100000000.00, 0.01, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", costAdj,
                            "digits", 2, NULL);
	column = gtk_tree_view_column_new_with_attributes(COST_COLUMN, renderer, "text", COST, NULL);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COSTF);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
		
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(COSTAVG_COLUMN, renderer, "text", COSTAVG, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COSTAVGF); 
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(COSTAVGTTL_COLUMN, renderer, "text", COSTAVGTTL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COSTAVGTTLF);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(PROFIT_COLUMN, renderer, "text", PROFIT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PROFITF); 
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(MARGIN_COLUMN, renderer, "text", MARGIN, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE); 
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, MARGINF); 
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*priceAdj;
	priceAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00, 100000000.00, 0.01, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", priceAdj,
                            "digits", 2, NULL);
	column = gtk_tree_view_column_new_with_attributes(PRICE_COLUMN, renderer, "text", PRICE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PRICEF);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedPrice), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	/*
	GtkAdjustment 	*dealerAdj;
	priceAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00, 100000000.00, 0.01, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", priceAdj,
                            "digits", 2, NULL);
	column = gtk_tree_view_column_new_with_attributes("Dealer", renderer, "text", DEALER, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column.
	gtk_tree_view_column_set_sort_column_id (column, DEALER);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDealer), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	*/
	
	column_counter++;
	
	renderer = gtk_cell_renderer_toggle_new ();
	column = gtk_tree_view_column_new_with_attributes(DEALER_COLUMN, renderer, "active", DEALER, NULL);
	gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DEALER);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);	
	g_signal_connect (G_OBJECT (renderer), "toggled", G_CALLBACK (cellClickedDealer), (gpointer) inventory);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*stockAdj;
	stockAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 100000000.00, 1.0, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", stockAdj,
                            "digits", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes(STOCK_COLUMN, renderer, "text", STOCK, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, STOCKF);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);	
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedStock), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	GtkAdjustment 	*lowStockAdj;
	lowStockAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.0, 0.0, 100000000.00, 1.0, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", lowStockAdj,
                            "digits", 0, NULL);
	column = gtk_tree_view_column_new_with_attributes(ORDER_COLUMN, renderer, "text", LOWSTOCKLVL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, LOWSTOCKLVLF);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);	
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedLowStockLVL), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(TOTALSOLD_COLUMN, renderer, "text", TOTAL_SOLD, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, TOTAL_SOLD);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(TOTALSOLDAMOUNT_COLUMN, renderer, "text", TOTAL_SOLD_AMOUNT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, TOTAL_SOLD_AMOUNTF); 
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("TTL Sold $ F", renderer, "text", TOTAL_SOLD_AMOUNTF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, TOTAL_SOLD_AMOUNTF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);			
		
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(LASTSOLD_COLUMN, renderer, "text", LAST_SOLD, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, LAST_SOLD);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);	 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);
	
	column_counter++;
	// These cells below are hidden and used for cell order organization only.
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("CostF", renderer, "text", COSTF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, COSTF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("CostAvg.F", renderer, "text", COSTAVGF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, COSTAVGF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("CostAvgTTL.F", renderer, "text", COSTAVGTTLF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, COSTAVGF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
		
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ProfitF", renderer, "text", PROFITF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, PROFITF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("MarginF", renderer, "text", MARGINF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, MARGINF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("PriceF", renderer, "text", PRICEF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, PRICEF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("StockF", renderer, "text", STOCKF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, STOCKF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("LowStockLVLF", renderer, "text", LOWSTOCKLVLF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, LOWSTOCKLVLF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Weight(kg)F", renderer, "text", WEIGHTF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, WEIGHTF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);		
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Length(mm)F", renderer, "text", LENGTHI, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, LENGTHI); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Width(mm)F", renderer, "text", WIDTHI, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, WIDTHI); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Height(mm)F", renderer, "text", HEIGHTI, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, HEIGHTI); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);		
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(yearTitle1, renderer, "text", YEARTOTAL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, YEARTOTALINT);
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("yearint", renderer, "text", YEARTOTALINT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, YEARTOTALINT); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);		
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(yearTitle2, renderer, "text", YEAREST, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, YEARESTINT); 
	gtk_tree_view_column_set_alignment(column, 0.5);
	g_object_set (renderer, "xalign", 0.5, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("yearestint", renderer, "text", YEARESTINT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, YEARESTINT); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(yearTitle3, renderer, "text", YEARTOTALAMOUNT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, YEARTOTALAMOUNTF); 
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("yearttlf", renderer, "text", YEARTOTALAMOUNTF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, YEARTOTALAMOUNTF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes(yearTitle4, renderer, "text", YEARESTAMOUNT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, YEARESTAMOUNTF); 
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);	
	
	column_counter++;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("yearamountf", renderer, "text", YEARESTAMOUNTF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, YEARESTAMOUNTF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	//g_free(yearTitle1), g_free(yearTitle2), g_free(yearTitle3), g_free(yearTitle4);		
}

/* Keyboard key press event on the tax rule delete window widget */
static gboolean deleteItem_key_press(GtkWidget *widget, GdkEventKey *ev, intrackInventory *inventory) {
	
    switch(ev->keyval)
    {
        case GDK_Y:
			prepareItemRemoval(NULL, inventory);
			destroyGTKWidget(NULL, widget);
            break;
            
        case GDK_y:
			prepareItemRemoval(NULL, inventory);
			destroyGTKWidget(NULL, widget);
            break;    
            
        case GDK_N:
			destroyGTKWidget(NULL, widget);
            break;                      
            
        case GDK_n:
			destroyGTKWidget(NULL, widget);
            break;             
    }
    
    return FALSE;
}

// Keyboard key press event on the inventory tree
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, intrackInventory *inventory) {
	
    switch(ev->keyval)
    {       
        case GDK_Up:
				// Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection.
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;

        case GDK_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;

        case GDK_KP_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;  
            
        case GDK_End:
				g_timeout_add (1, keyPressSelection, (gpointer) inventory); 		
            break;
            		
        case GDK_Delete:
				deleteItemWindow(NULL, inventory);
			break;
			
		case GDK_KP_Delete:
				deleteItemWindow(NULL, inventory);
			break;                      
    }
	
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {

	intrackInventory *inventory = (intrackInventory *)data;

	if(gtk_tree_selection_count_selected_rows(inventory->selection) == 0) {
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
	}
	else if(gtk_tree_selection_count_selected_rows(inventory->selection) > 1) {
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
		gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
	}
	else {
		//gtk_widget_set_sensitive(inventory->invenMenuView, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, TRUE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, TRUE);
		gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->editItemButton, TRUE);
		
		GtkTreeSelection *selection;
		GtkTreeRowReference *ref;
		GtkTreeModel *model;
		GList *rows, *ptr, *references = NULL;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
		rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
		ptr = rows;
		// Create tree row references to all of the selected rows
		while(ptr != NULL) {
			ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
			references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
			gtk_tree_row_reference_free(ref);
			ptr = ptr->next;
		}

		g_list_foreach(references, (GFunc) keyPressGetRow, inventory);

		// Free the tree paths, tree row references and lists
		g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
		g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
		g_list_free(references);
		g_list_free(rows);
		
	}

	return FALSE;	
}

// Get the selected row from a key press (part #2)
static void keyPressGetRow(GtkTreeRowReference *ref, intrackInventory *inventory) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *rowBarcode;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &rowBarcode, -1);
    
    g_free(inventory->selectedItemCode);
    inventory->selectedItemCode = g_strconcat(rowBarcode, NULL);
    g_free(rowBarcode);
}

// Mouse click event on the tree
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackInventory *inventory) {

    GtkTreePath *path;
    
	// if there's no path where the click occurred...
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(inventory->inventoryTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
        return FALSE;
	}

	// Set the sensitivity on the viewItem Button
	g_timeout_add(1, selectionTimer, (gpointer) inventory); 	

    // Left mouse button click
    switch(ev->button)
    {
        case 1: // 1 = left click
            break;

        case 3: // 3 = right click
				gtk_menu_popup(GTK_MENU(inventory->invenMenu), NULL, NULL, NULL, NULL, 3, ev->time);
            break;
    }
    
	// Keep track of the selected item
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gchar *rowBarcode;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &rowBarcode, -1);
    
    g_free(inventory->selectedItemCode);
    inventory->selectedItemCode = g_strconcat(rowBarcode, NULL);
    g_free(rowBarcode);
    
    // free our path
    gtk_tree_path_free(path);  

    return FALSE;	
}

static gboolean selectionTimer(gpointer data) {
	
	intrackInventory *inventory = (intrackInventory *)data;

	/* If the user selects multiple rows in the basket tree, then turn off the itemView button */
	if(gtk_tree_selection_count_selected_rows(inventory->selection) > 1) {
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);	
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
		gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
	}
	else {
		//gtk_widget_set_sensitive(inventory->invenMenuView, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, TRUE);		
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, TRUE);
		gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->editItemButton, TRUE);
	}		
	
	return FALSE;
}

static void destroyGTKWidget(GtkWidget *widget, gpointer parentWindow) {
	gtk_widget_destroy(GTK_WIDGET(parentWindow));
}

static void hideGtkWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_hide_all(GTK_WIDGET(data));
}

/* Also a seg fault under a rare circumstance on the result fetch? */
/* This needs to be fixed. Looked at accounts.c checkAccountsExist() for proper implementation */
static int checkIfExist(gchar *barCode) {

	/* Connection Error */
	if(connectToServer() == 1)
		return 1;
	
	gchar *query_string;

	query_string = g_strconcat("SELECT partNo, stock FROM `", mysqlTables, "` WHERE partNo = '", barCode, "'", NULL);
	
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
	
		// Set global variable item does not exist.
		//itemNotExist = 0;

		gchar *messageText = g_strconcat(barCode, " Item Not Found", NULL);
		//printMessage(messageText, NULL);
		g_free(messageText);

		return 1;
	}
	
	//itemNotExist = 1;

	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);	

	return 0;
}

static void calculateTreeTotals(GtkTreeStore *store, intrackInventory *inventory) {

	GtkTreeIter iter;
	gchar *rowtext, **split, *join;
	//gchar *coststext, **splitcosts, *joincosts;
	//gchar *profitstext, **splitprofits, *joinprofits;
	int numberOfParts = 0;
	int tempStock = 0;
	float partCosts = 0;
	float partCostsAvg = 0;
	
	gint intTemp;
	gfloat floatTemp;
	
	gint ytdInt = 0;
	gfloat ytd = 0.00;
	gfloat ytdEst = 0.00;

	// Calculate the total sales, and the total sales amount in the latest tree starting from the very first row in the tree
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
		// Number Of Parts
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, STOCKF, &intTemp, -1);
		tempStock = intTemp;
		numberOfParts = numberOfParts + intTemp;
		
		// Parts Costs
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, COSTF, &floatTemp, -1);
		partCosts = partCosts + (floatTemp * tempStock);
		
		// Parts Costs Average
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, COSTAVGF, &floatTemp, -1);
		partCostsAvg = partCostsAvg + (floatTemp * tempStock);
		
		// YTD # of items sold
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, YEARTOTALINT, &intTemp, -1);
		ytdInt = ytdInt + intTemp;
		
		// YTD sales
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, YEARTOTALAMOUNTF, &floatTemp, -1);
		ytd = ytd + floatTemp;
		
		// YTD EST sales
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, YEARESTAMOUNTF, &floatTemp, -1);
		ytdEst = ytdEst + floatTemp;
		
		// Finish off calculating the rest of the rows in the tree
		while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)) {
			// Number Of Parts
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, STOCKF, &intTemp, -1);
			tempStock = intTemp;
			numberOfParts = numberOfParts + intTemp;
		
			// Parts Costs
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, COSTF, &floatTemp, -1);
			partCosts = partCosts + (floatTemp * tempStock);
		
			// Parts Costs Average
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, COSTAVGF, &floatTemp, -1);
			partCostsAvg = partCostsAvg + (floatTemp * tempStock);
			
			// YTD # of items sold
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, YEARTOTALINT, &intTemp, -1);
			ytdInt = ytdInt + intTemp;
			
			// YTD sales
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, YEARTOTALAMOUNTF, &floatTemp, -1);
			ytd = ytd + floatTemp;
			
			// YTD EST sales
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, YEARESTAMOUNTF, &floatTemp, -1);
			ytdEst = ytdEst + floatTemp;
		}
	}

    char *locale;
    locale = setlocale(LC_NUMERIC, "en_US.iso88591");
    // printf("%'d\n", 12345);
    setlocale(LC_NUMERIC, locale);
    
    struct tm *ptr;
    time_t lt;
    char str[80];
	char *text;
	
    lt = time(NULL);
    ptr = localtime(&lt);

    strftime(str, 100, "%G", ptr);    
        
	gchar *numberOfPartsChar = NULL;
	gchar *partCostsChar = NULL;
	gchar *partCostsAvgChar = NULL;
	gchar *ytdItemsChar, *ytdSalesChar, *ytdEstSalesChar;
	
	numberOfPartsChar = g_strconcat("Items in stock: ", g_strdup_printf("%i", numberOfParts), NULL);
	partCostsChar = g_strconcat("Total Item Cost: ", g_strdup_printf("%'0.2f", partCosts), NULL);
	partCostsAvgChar = g_strconcat("Total Average Item Cost: ", g_strdup_printf("%'0.2f", partCostsAvg), NULL);

	ytdItemsChar = g_strconcat(str, " YTD: ", g_strdup_printf("%i", ytdInt), NULL);
	ytdSalesChar = g_strconcat(str, " YTD Sales: ", g_strdup_printf("%'0.2f", ytd), NULL);
	ytdEstSalesChar = g_strconcat(str, " YTD Est Sales: ", g_strdup_printf("%'0.2f", ytdEst), NULL);

	gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabel), numberOfPartsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partCostsLabel), partCostsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partCostsAvgLabel), partCostsAvgChar);

	gtk_label_set_text(GTK_LABEL(inventory->ytdItemsLabel), ytdItemsChar);
	gtk_label_set_text(GTK_LABEL(inventory->ytdSalesLabel), ytdSalesChar);
	gtk_label_set_text(GTK_LABEL(inventory->ytdEstSalesLabel), ytdEstSalesChar);

	g_free(numberOfPartsChar), g_free(partCostsChar), g_free(partCostsAvgChar), g_free(ytdItemsChar), g_free(ytdSalesChar), g_free(ytdEstSalesChar);
}

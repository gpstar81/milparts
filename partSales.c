//      partSales.c
//      Copyright 2010 - 2011 - 2012 - 2013 - 2014 Michael Rajotte <michael@michaelrajotte.com>
// 		For inventory control, mass listing, searching and editing via a gtktree.

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
#include "partSales.h"

/*
TODO:
(1). Build serial # recording.
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

(2). Things to add to items:
	-Last ordered -> This can be put in orders.c
	-Next Arrival -> This can be put in orders.c
	-On Order -> This can be put in orders.c
	-Weight
	-Location
	-Inventory Value
(3). checkIfExist() needs to be fixed. Look at accounts.c checkAccountExist() for proper implementation.

#### Color Coding Table ####
Red = No Stock.
Yellow = Stock is below stock low level indicator.
Purple = Serial numbers missing.
*/

void initalizePartSales(GtkBuilder *builder, gchar *mysqlDatabase, gchar *inventoryTable, GtkWidget *mainWindow) {
	
	/* Load up the old widgets. <-- This is old stuff, delete after when rewriting new code */
	//initalizeWidgets(builder);
	
	/* Initialize the intrackInventory structure */
	intrackInventory *inventory;
	inventory = (intrackInventory*) g_malloc (sizeof (intrackInventory)); // Allocate memory for the data.
	
	inventory->mysqlDatabase = g_strconcat(mysqlDatabase, NULL);
	inventory->inventoryTable = g_strconcat(inventoryTable, NULL);
	inventory->mainWindow = mainWindow;
	inventory->selectedItemCode	 = g_strconcat(NULL, NULL);
	inventory->exportQueryString = g_strconcat(NULL, NULL);
	
	// Setup the selection numbers.
	inventory->numberOfParts = 0;
	inventory->partCosts = 0;
	inventory->partSales = 0;
	inventory->partProfits = 0;
	inventory->partMargin = 0;
	
	// Setup the difference numbers.
	inventory->numberOfPartsPrev = 0;
	inventory->partCostsPrev = 0;
	inventory->partSalesPrev = 0;
	inventory->partProfitsPrev = 0;
	inventory->partMarginPrev = 0;
	
    // Exit program button from file / quit.
    GtkWidget	*exitInventory;
    exitInventory = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryCloseButton"));
   	g_signal_connect(G_OBJECT(exitInventory), "activate", G_CALLBACK(hideGtkWidget), mainWindow);		
   	
   	/* Toolbar buttons */
   	/*
   	GtkWidget	*addButton;
   	addButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenAddButton"));
   	g_signal_connect(G_OBJECT(addButton), "clicked", G_CALLBACK(prepareAddItem), mainWindow);
   	
   	GtkWidget	*categoryButton;
   	categoryButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenCategoryButton"));
   	g_signal_connect(G_OBJECT(categoryButton), "clicked", G_CALLBACK(openCategories), mainWindow);
   	*/
   	/*
   	inventory->removeItemButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenDelButton"));
   	g_signal_connect(G_OBJECT(inventory->removeItemButton), "clicked", G_CALLBACK(deleteItemWindow), inventory);
   	gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
   	*/
   	/*
   	inventory->invenViewItemButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenViewItemButton"));
   	g_signal_connect(G_OBJECT(inventory->invenViewItemButton), "clicked", G_CALLBACK(prepareViewWindow), inventory);   	
   	gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
   	
   	inventory->editItemButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenEditButton"));
   	g_signal_connect(G_OBJECT(inventory->editItemButton), "clicked", G_CALLBACK(prepareEditWindow), inventory);
   	gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
   	*/
   	
   	GtkWidget	*addButton;
   	addButton = GTK_WIDGET(gtk_builder_get_object(builder, "addSaleButton"));
   	g_signal_connect(G_OBJECT(addButton), "clicked", G_CALLBACK(prepareSaleAddItem), inventory);
   	   	
   	GtkWidget	*exportButton;
   	exportButton = GTK_WIDGET(gtk_builder_get_object(builder, "exportInventoryButton"));
   	g_signal_connect(G_OBJECT(exportButton), "clicked", G_CALLBACK(prepareExport), inventory);
   	
   	inventory->returnButton = GTK_WIDGET(gtk_builder_get_object(builder, "returnButton"));
   	g_signal_connect(G_OBJECT(inventory->returnButton), "clicked", G_CALLBACK(deleteItemWindow), inventory);
   	gtk_widget_set_sensitive(inventory->returnButton, FALSE);
   	
   	/*
   	GtkWidget	*importButton;
   	importButton = GTK_WIDGET(gtk_builder_get_object(builder, "invenImportButton"));
   	g_signal_connect(G_OBJECT(importButton), "clicked", G_CALLBACK(prepareImport), inventory);
	*/
	
   	// Information total labels
   	inventory->numberOfPartsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "numberOfPartsLabel"));
   	inventory->partCostsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsLabel"));
   	inventory->partSalesLabel = GTK_WIDGET(gtk_builder_get_object(builder, "partSalesLabel"));
   	inventory->partProfitsLabel = GTK_WIDGET(gtk_builder_get_object(builder, "partProfitsLabel"));
   	inventory->partMarginLabel = GTK_WIDGET(gtk_builder_get_object(builder, "partMarginLabel"));
   	
   	// Selection information total labels
   	inventory->numberOfPartsLabel1 = GTK_WIDGET(gtk_builder_get_object(builder, "numberOfPartsLabel1"));
   	inventory->partCostsLabel1 = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsLabel1"));
   	inventory->partSalesLabel1 = GTK_WIDGET(gtk_builder_get_object(builder, "partSalesLabel1"));
   	inventory->partProfitsLabel1 = GTK_WIDGET(gtk_builder_get_object(builder, "partProfitsLabel1"));   	
   	inventory->partMarginLabel1 = GTK_WIDGET(gtk_builder_get_object(builder, "partMarginLabel1"));   	
   	
   	// Previous Year Information total labels
   	inventory->numberOfPartsLabelPrev = GTK_WIDGET(gtk_builder_get_object(builder, "numberOfPartsLabelPrev"));
   	inventory->partCostsLabelPrev = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsLabelPrev"));
   	inventory->partSalesLabelPrev = GTK_WIDGET(gtk_builder_get_object(builder, "partSalesLabelPrev"));
   	inventory->partProfitsLabelPrev = GTK_WIDGET(gtk_builder_get_object(builder, "partProfitsLabelPrev"));
   	inventory->partMarginLabelPrev = GTK_WIDGET(gtk_builder_get_object(builder, "partMarginLabelPrev"));
   	
   	// Difference Information total labels
   	inventory->numberOfPartsLabelDiff = GTK_WIDGET(gtk_builder_get_object(builder, "numberOfPartsLabelDiff"));
   	inventory->partCostsLabelDiff = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsLabelDiff"));
   	inventory->partSalesLabelDiff = GTK_WIDGET(gtk_builder_get_object(builder, "partSalesLabelDiff"));
   	inventory->partProfitsLabelDiff = GTK_WIDGET(gtk_builder_get_object(builder, "partProfitsLabelDiff"));
   	inventory->partMarginLabelDiff = GTK_WIDGET(gtk_builder_get_object(builder, "partMarginLabelDiff"));
   	
   	// % increase/decrease information total labels
   	inventory->numberOfPartsLabelPerc = GTK_WIDGET(gtk_builder_get_object(builder, "numberOfPartsLabelPerc"));
   	inventory->partCostsLabelPerc = GTK_WIDGET(gtk_builder_get_object(builder, "partCostsLabelPerc"));
   	inventory->partSalesLabelPerc = GTK_WIDGET(gtk_builder_get_object(builder, "partSalesLabelPerc"));
   	inventory->partProfitsLabelPerc = GTK_WIDGET(gtk_builder_get_object(builder, "partProfitsLabelPerc"));   
   	inventory->partMarginLabelPerc = GTK_WIDGET(gtk_builder_get_object(builder, "partMarginLabelPerc"));   	   	

    /* Close the window */
    GtkWidget	*inventoryCloseButtonBar;
    inventoryCloseButtonBar = GTK_WIDGET(gtk_builder_get_object(builder, "inventoryCloseButtonBar"));
   	g_signal_connect(G_OBJECT(inventoryCloseButtonBar), "clicked", G_CALLBACK(hideGtkWidget), mainWindow);	   	
	
	/* Setup the inventory tree */
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
	
	/* Now set the tree so it can handle multiple selections */
	//GtkTreeSelection *selection;
	inventory->selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (inventory->inventoryTree));
	gtk_tree_selection_set_mode (inventory->selection, GTK_SELECTION_MULTIPLE);
	
	/* Setup the inventory search checkbox buttons */
	inventory->barcodeSearch = GTK_WIDGET(gtk_builder_get_object(builder, "barcodeSearch"));
	inventory->descriptionSearch = GTK_WIDGET(gtk_builder_get_object(builder, "descriptionSearch"));
	inventory->soldToSearch = GTK_WIDGET(gtk_builder_get_object(builder, "soldToSearch"));
	inventory->costSearch = GTK_WIDGET(gtk_builder_get_object(builder, "costSearch")); /* Belongs to number search */
	inventory->priceSearch = GTK_WIDGET(gtk_builder_get_object(builder, "priceSearch")); /* Belongs to number search */
	
	/* Setup popup menu when right clicking on the inventory tree */
	//inventory->invenMenu = gtk_menu_new();
	
	/*
	inventory->invenMenuView = gtk_menu_item_new_with_label("View item");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuView);
	gtk_widget_show(inventory->invenMenuView);
	g_signal_connect(G_OBJECT(inventory->invenMenuView), "activate", G_CALLBACK(prepareViewWindow), inventory);
	gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
	
	inventory->invenMenuEdit = gtk_menu_item_new_with_label("Edit item");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuEdit);
	gtk_widget_show(inventory->invenMenuEdit);
	g_signal_connect(G_OBJECT(inventory->invenMenuEdit), "activate", G_CALLBACK(prepareEditWindow), inventory);
	gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);	
	*/
	/*
	inventory->invenMenuRemove = gtk_menu_item_new_with_label("Remove item");
	gtk_menu_shell_append(GTK_MENU_SHELL(inventory->invenMenu), inventory->invenMenuRemove);
	gtk_widget_show(inventory->invenMenuRemove);
	g_signal_connect(G_OBJECT(inventory->invenMenuRemove), "activate", G_CALLBACK(deleteItemWindow), inventory);
	*/
	
	/* Setup keypress signals on the tree */
	g_signal_connect(inventory->inventoryTree, "button-press-event", G_CALLBACK(treeButtonPress), inventory);
	g_signal_connect(inventory->inventoryTree, "key-press-event", G_CALLBACK(treeKeyPress), inventory);
	
	/* Load the inventory tree with all data from the database */
	//getInventory(NULL, inventory);
	g_signal_connect(inventory->mainWindow, "show", G_CALLBACK(getInventory), inventory);
	
/* ------------------------------------------------------------------------------------------------------------------------------ */
	/* Creates the FROM calendar widget and entry box */
	inventory->dateEntryFrom = (GtkDateEntry*) g_malloc (sizeof (GtkDateEntry)); // Allocate memory for the data.
	inventory->dateEntryFrom->date = g_date_new();

	/* today's date */
	g_date_set_time_t(inventory->dateEntryFrom->date, time(NULL));
	g_date_set_dmy(&inventory->dateEntryFrom->mindate, 1, 1, 1900);
	g_date_set_dmy(&inventory->dateEntryFrom->maxdate, 31, 12, 2200);
	g_date_subtract_days(inventory->dateEntryFrom->date, 30);

	// Entry and button for date from
	gchar	dateBuffer[256];
	g_date_strftime(dateBuffer, 256, "%Y-%m-%d", inventory->dateEntryFrom->date);

	inventory->dateEntryFrom->entry = GTK_WIDGET(gtk_builder_get_object(builder, "dateEntryFrom"));
	inventory->dateEntryFrom->arrow = GTK_WIDGET(gtk_builder_get_object(builder, "fromToggle"));

	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->arrow), "toggled", G_CALLBACK (gtk_dateentry_arrow_press), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->entry), "activate", G_CALLBACK (gtk_dateentry_entry_new), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->entry), "focus-out-event", G_CALLBACK (gtk_dateentry_focus), inventory->dateEntryFrom);

	/* our popup window */
	inventory->dateEntryFrom->popwin = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_events (inventory->dateEntryFrom->popwin, gtk_widget_get_events(inventory->dateEntryFrom->popwin) | GDK_KEY_PRESS_MASK);

	inventory->dateEntryFrom->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (inventory->dateEntryFrom->popwin), inventory->dateEntryFrom->frame);
	gtk_frame_set_shadow_type (GTK_FRAME (inventory->dateEntryFrom->frame), GTK_SHADOW_OUT);
	gtk_widget_show (inventory->dateEntryFrom->frame);

	inventory->dateEntryFrom->calendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (inventory->dateEntryFrom->frame), inventory->dateEntryFrom->calendar);
	gtk_widget_show (inventory->dateEntryFrom->calendar);

	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->popwin), "key_press_event", G_CALLBACK (key_press_popup), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->popwin), "button_press_event", G_CALLBACK (gtk_dateentry_button_press), inventory->dateEntryFrom);

	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->calendar), "prev-year", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->calendar), "next-year", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->calendar), "prev-month", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->calendar), "next-month", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryFrom);

	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->calendar), "day-selected", G_CALLBACK (gtk_dateentry_calendar_getfrom), inventory->dateEntryFrom);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryFrom->calendar), "day-selected-double-click", G_CALLBACK (gtk_dateentry_calendar_select), inventory->dateEntryFrom);

	gtk_dateentry_calendar_getfrom(NULL, inventory->dateEntryFrom);
	gtk_entry_set_text(GTK_ENTRY(inventory->dateEntryFrom->entry), dateBuffer);
	/* ------------------------------------------------------------------------------------------------------------------------------ */	
	
	/* ------------------------------------------------------------------------------------------------------------------------------ */
	/* Creates the TO calendar widget and entry box */
	inventory->dateEntryTo = (GtkDateEntry*) g_malloc (sizeof (GtkDateEntry)); // Allocate memory for the data.
	inventory->dateEntryTo->date = g_date_new();
	
	/* today's date */
	g_date_set_time_t(inventory->dateEntryTo->date, time(NULL));
	g_date_set_dmy(&inventory->dateEntryTo->mindate, 1, 1, 1900);
	g_date_set_dmy(&inventory->dateEntryTo->maxdate, 31, 12, 2200);

	// Entry and button for date to
	inventory->dateEntryTo->entry = GTK_WIDGET(gtk_builder_get_object(builder, "dateEntryTo"));
	inventory->dateEntryTo->arrow = GTK_WIDGET(gtk_builder_get_object(builder, "toToggle"));

	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->arrow), "toggled", G_CALLBACK (gtk_dateentry_arrow_press), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->entry), "activate", G_CALLBACK (gtk_dateentry_entry_new), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->entry), "focus-out-event", G_CALLBACK (gtk_dateentry_focus), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->entry), "key_press_event", G_CALLBACK (gtk_dateentry_entry_key), inventory->dateEntryTo);

	/* our popup window */
	inventory->dateEntryTo->popwin = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_widget_set_events (inventory->dateEntryTo->popwin, gtk_widget_get_events(inventory->dateEntryTo->popwin) | GDK_KEY_PRESS_MASK);

	inventory->dateEntryTo->frame = gtk_frame_new (NULL);
	gtk_container_add (GTK_CONTAINER (inventory->dateEntryTo->popwin), inventory->dateEntryTo->frame);
	gtk_frame_set_shadow_type (GTK_FRAME (inventory->dateEntryTo->frame), GTK_SHADOW_OUT);
	gtk_widget_show (inventory->dateEntryTo->frame);

	inventory->dateEntryTo->calendar = gtk_calendar_new ();
	gtk_container_add (GTK_CONTAINER (inventory->dateEntryTo->frame), inventory->dateEntryTo->calendar);
	gtk_widget_show (inventory->dateEntryTo->calendar);

	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->popwin), "key_press_event", G_CALLBACK (key_press_popup), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->popwin), "button_press_event", G_CALLBACK (gtk_dateentry_button_press), inventory->dateEntryTo);

	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->calendar), "prev-year", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->calendar), "next-year", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->calendar), "prev-month", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->calendar), "next-month", G_CALLBACK (gtk_dateentry_calendar_year), inventory->dateEntryTo);

	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->calendar), "day-selected", G_CALLBACK (gtk_dateentry_calendar_getfrom), inventory->dateEntryTo);
	g_signal_connect (GTK_OBJECT (inventory->dateEntryTo->calendar), "day-selected-double-click", G_CALLBACK (gtk_dateentry_calendar_select), inventory->dateEntryTo);

	gtk_dateentry_calendar_getfrom(NULL, inventory->dateEntryTo);
	/* ------------------------------------------------------------------------------------------------------------------------------ */	
}

/* Import a raw .csv file to the database. -> import.c */
static void prepareImport(GtkWidget *widget, intrackInventory *inventory) {
	//importDatabase(inventory->mainWindow); /* import.c */
}

/* Export the inventory database. -> export.c */
static void prepareExport(GtkWidget *widget, intrackInventory *inventory) {
	mysqlExportSales(NULL, inventory->mainWindow, inventory->exportQueryString); /* export.c */
}

/* Opens a widget popup to prompt user to add a item. -> partSales_add.c */
static void prepareSaleAddItem(GtkWidget *widget, intrackInventory *inventory) {
	initalizeSaleAdd(inventory->mainWindow); // -> partSales_add.c
}

/* Opens the serial number editor. -> inventory_serial.c */
static void prepareSerialWindow(GtkWidget *widget, intrackInventory *inventory) {
	//initalizeSerialEditor(inventory->mainWindow, inventory->selectedItemCode);
}

/* Opens the category editor window. -> inventory_cat.c */
static void openCategories(GtkWidget *widget, GtkWidget *mainWindow) {
	//initalizeCategories(mainWindow); // -> inventory_cat.c
}

/* Open up a window which allows full editing of a item. -> inventory_edit.c */
static void prepareEditWindow(GtkWidget *widget, intrackInventory *inventory) {
	//initalizeEditor(inventory->mainWindow, inventory->inventoryTree, inventory->selectedItemCode, &inventory->serial, inventory->invenMenuSerialButton, inventory->invenSerialButton); // -> inventory_edit.c
}

/* Open up a window which shows the item information (picture, info, stats, etc). -> view_item.c */
static void prepareViewWindow(GtkWidget *widget, intrackInventory *inventory) {
	//loadViewItem(inventory->mainWindow, inventory->selectedItemCode); // -> view_item.c
}

/* Opens a widget popup to prompt user to add a item. -> inventory_add.c */
static void prepareAddItem(GtkWidget *widget, GtkWidget *mainWindow) {
	//initalizeAdd(mainWindow); // -> inventory_add.c
}

/* A widget window to ask for confirmation upon deletion */
static void deleteItemWindow(GtkWidget *widget, intrackInventory *inventory) {
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(inventory->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Remove selected items and return to stock?");
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Return to stock?");
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

/* A widget window to ask for confirmation upon setting future dates */
static void setDateWindow(GtkWidget *widget, intrackInventory *inventory) {
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(inventory->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Set Date into the future?");
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Set Date Into Future?");
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

/* Pulls inventory data when searching by numbers ie: cost, price and stock */
static void getInventoryNumbers(GtkWidget *widget, intrackInventory *inventory) {
	/* Clear out selected item before refreshing the tree */
	g_free(inventory->selectedItemCode);
	inventory->selectedItemCode = g_strconcat(NULL, NULL);
   	//gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
    //gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
   	//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
   	gtk_widget_set_sensitive(inventory->returnButton, FALSE);
   		
	GtkListStore *store;
	
	store = gtk_list_store_new(INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT);
	
		GDate *dateFrom = g_date_new();
		GDate *dateTo = g_date_new();
		
		processDate(dateFrom, inventory->dateEntryFrom->entry); // functions.h
		processDate(dateTo, inventory->dateEntryTo->entry); // functions.h		

	float min, max;
	
	min = gtk_spin_button_get_value(GTK_SPIN_BUTTON(inventory->searchSpinMin));
	max = gtk_spin_button_get_value(GTK_SPIN_BUTTON(inventory->searchSpinMax));
	
	if(max >= min) {
		gchar *minChar, *maxChar;
		minChar = g_strdup_printf("%f", min);
		maxChar = g_strdup_printf("%f", max);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->costSearch)))
			pullInventory(inventory, store, inventory->inventoryTable, "cost", NULL, minChar, maxChar, dateFrom, dateTo, FALSE, FALSE);

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->priceSearch)))
			pullInventory(inventory, store, inventory->inventoryTable, "price", NULL, minChar, maxChar, dateFrom, dateTo, FALSE, FALSE);
			
		g_free(minChar);
		g_free(maxChar);
	}
	else
		printMessage(ERROR_SEARCH_NUMBER, inventory->mainWindow);

	gtk_tree_view_set_model(GTK_TREE_VIEW(inventory->inventoryTree), GTK_TREE_MODEL(store));
		
	calculateTreeTotals(store, inventory);
	
	g_object_unref(store);
	g_date_free(dateFrom), g_date_free(dateTo);	
}

/* Prepare to pull inventory data from the database */
static void getInventory(GtkWidget *widget, intrackInventory *inventory) {
	/* Clear out selected item before refreshing the tree */
	g_free(inventory->selectedItemCode);
	inventory->selectedItemCode = g_strconcat(NULL, NULL);
   	//gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
   	//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
   	//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
   	gtk_widget_set_sensitive(inventory->returnButton, FALSE);
   	
	GtkListStore *store;
	
	store = gtk_list_store_new(INVENTORY_COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT, G_TYPE_FLOAT);
	
		GDate *dateFrom = g_date_new();
		GDate *dateTo = g_date_new();
		
		processDate(dateFrom, inventory->dateEntryFrom->entry); // functions.h
		processDate(dateTo, inventory->dateEntryTo->entry); // functions.h	
		
	/* Checks to see if a ' is entered, that causes mysql.h to break */
	if(g_strrstr(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry)), "'")) {
		printMessage("ERROR: ' not allowed.", inventory->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(inventory->inventorySearchEntry), ""); /* Clear out the search entry */
	}
	/* This will search the inventory for the user query if the search entry is greater than 2 characters */
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry))) > 2) {
		gchar *searchString;
		searchString = g_strconcat(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry)), NULL);		
		
		float min, max;
		
		min = gtk_spin_button_get_value(GTK_SPIN_BUTTON(inventory->searchSpinMin));
		max = gtk_spin_button_get_value(GTK_SPIN_BUTTON(inventory->searchSpinMax));
		
		if(max >= min) {
			gchar *minChar, *maxChar;
			minChar = g_strdup_printf("%f", min);
			maxChar = g_strdup_printf("%f", max);

			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->costSearch))) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->barcodeSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "partNo", searchString, minChar, maxChar, dateFrom, dateTo, TRUE, FALSE);
					
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->descriptionSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "partDesc", searchString, minChar, maxChar, dateFrom, dateTo, TRUE, FALSE);
					
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->soldToSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "soldTo", searchString, minChar, maxChar, dateFrom, dateTo, TRUE, FALSE);	
					
				pullInventory(inventory, store, inventory->inventoryTable, "invoiceNo", searchString, minChar, maxChar, dateFrom, dateTo, TRUE, FALSE);					
			}

			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->priceSearch))) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->barcodeSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "partNo", searchString, minChar, maxChar, dateFrom, dateTo, FALSE, TRUE);
					
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->descriptionSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "partDesc", searchString, minChar, maxChar, dateFrom, dateTo, FALSE, TRUE);
					
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->soldToSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "soldTo", searchString, minChar, maxChar, dateFrom, dateTo, FALSE, TRUE);
					
				pullInventory(inventory, store, inventory->inventoryTable, "invoiceNo", searchString, minChar, maxChar, dateFrom, dateTo, FALSE, TRUE);	
			}
			
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->priceSearch)) == FALSE && gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->costSearch)) == FALSE) {
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->barcodeSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "partNo", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);
					
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->descriptionSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "partDesc", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);
					
				if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->soldToSearch)))
					pullInventory(inventory, store, inventory->inventoryTable, "soldTo", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);
					
				pullInventory(inventory, store, inventory->inventoryTable, "invoiceNo", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);						
			}
				
			g_free(minChar);
			g_free(maxChar);
		}
		else {
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->barcodeSearch)))
				pullInventory(inventory, store, inventory->inventoryTable, "partNo", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);
				
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->descriptionSearch)))
				pullInventory(inventory, store, inventory->inventoryTable, "partDesc", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);
				
			if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(inventory->soldToSearch)))
				pullInventory(inventory, store, inventory->inventoryTable, "soldTo", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);	
				
			pullInventory(inventory, store, inventory->inventoryTable, "invoiceNo", searchString, NULL, NULL, dateFrom, dateTo, FALSE, FALSE);							
		
		}
	
		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(inventory->inventorySearchEntry), "");
	}
	/* If the search entry is less than 3 characters but greater than 0 */
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(inventory->inventorySearchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, inventory->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(inventory->inventorySearchEntry), "");
	}
	else
		pullInventory(inventory, store, inventory->inventoryTable, NULL, NULL, NULL, NULL, dateFrom, dateTo, FALSE, FALSE); /* This pulls all the inventory out of the database */
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(inventory->inventoryTree), GTK_TREE_MODEL(store));
	
	calculateTreeTotals(store, inventory);
	
	g_object_unref(store);
	g_date_free(dateFrom), g_date_free(dateTo);	
}

/* Pulls the inventory data out of the database and stores it into a GtkListStore */
static int pullInventory(intrackInventory *inventory, GtkListStore *store, gchar *inventoryTable, gchar *searchWhere, gchar *searchString, gchar *searchNumMin, gchar *searchNumMax, GDate *dateFrom, GDate *dateTo, gboolean costPull, gboolean pricePull) {
	GtkTreeIter 	iter;
	
	int i;
	int num_fields;	
	gchar *query_string;
	int query_state;	
	
	MYSQL *partsConnection, partsMysql;
	MYSQL_RES *partsResult;
	MYSQL_ROW partsRow;		
	
	mysql_init(&partsMysql);
	
	/* Open connection to the database */
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
	
	gchar *searchDateFrom;
	gchar *searchDateTo;
	gchar *searchDateCompFrom, *searchDateCompTo;
	
	gchar *yearFromChar, *monthFromChar, *dayFromChar;
	gchar *yearToChar, *monthToChar, *dayToChar;
	gchar *yearFromCompChar, *yearToCompChar;
	
	yearToChar = g_strdup_printf("%i", dateTo->year);
	monthToChar = g_strdup_printf("%i", dateTo->month);
	dayToChar = g_strdup_printf("%i", dateTo->day);
		
	yearFromChar = g_strdup_printf("%i", dateFrom->year);
	monthFromChar = g_strdup_printf("%i", dateFrom->month);
	dayFromChar= g_strdup_printf("%i", dateFrom->day);
	
	yearToCompChar = g_strdup_printf("%i", dateTo->year - 1);
	yearFromCompChar = g_strdup_printf("%i", dateFrom->year - 1);
	
	searchDateFrom = g_strconcat(yearFromChar, "-", monthFromChar, "-", dayFromChar, " 00:00:00", NULL);
	searchDateTo = g_strconcat(yearToChar, "-", monthToChar, "-", dayToChar, " 23:59:59", NULL);
	
	searchDateCompFrom = g_strconcat(yearFromCompChar, "-", monthFromChar, "-", dayFromChar, " 00:00:00", NULL);
	searchDateCompTo = g_strconcat(yearToCompChar, "-", monthToChar, "-", dayToChar, " 23:59:59", NULL);	
	
	g_free(yearFromChar);
	g_free(monthFromChar);
	g_free(dayFromChar);
	g_free(yearToChar);
	g_free(monthToChar); 
	g_free(dayToChar);
	g_free(yearToCompChar);
	g_free(yearFromCompChar);
		
	// Search for the user entered query
	if(searchString != NULL) {
		if(searchNumMin != NULL && searchNumMax != NULL) {
			if(costPull == TRUE)
				query_string = g_strconcat("SELECT id, partNo, partDesc, cost, price, dateSold, soldTo, invoiceNo FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateFrom, "' AND dateSold < '", searchDateTo, "' AND cost >= ", searchNumMin, " AND cost <= ", searchNumMax, " AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);				
			else if(pricePull == TRUE)
				query_string = g_strconcat("SELECT id, partNo, partDesc, cost, price, dateSold, soldTo, invoiceNo FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateFrom, "' AND dateSold < '", searchDateTo, "' AND price >= ", searchNumMin, " AND price <= ", searchNumMax, " AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);			
			else
				query_string = g_strconcat("SELECT id, partNo, partDesc, cost, price, dateSold, soldTo, invoiceNo FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateFrom, "' AND dateSold < '", searchDateTo, "' AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);
		} 
		else
			query_string = g_strconcat("SELECT id, partNo, partDesc, cost, price, dateSold, soldTo, invoiceNo FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateFrom, "' AND dateSold < '", searchDateTo, "' AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);
	}
	else if(searchNumMin != NULL && searchNumMax != NULL)
		query_string = g_strconcat("SELECT id, partNo, partDesc, cost, price, dateSold, soldTo, invoiceNo FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateFrom, "' AND dateSold < '", searchDateTo, "' AND ", searchWhere, " >= ", searchNumMin, " AND ", searchWhere, " <= ", searchNumMax, " ORDER BY dateSold DESC", NULL);
	else
		query_string = g_strconcat("SELECT id, partNo, partDesc, cost, price, dateSold, soldTo, invoiceNo FROM `", SOLD_TABLES, "` WHERE dateSold > '", searchDateFrom, "' AND dateSold < '", searchDateTo, "' ORDER BY dateSold DESC", NULL);	

	mysql_query(partsConnection, query_string);
	
	g_free(searchDateTo);
	g_free(searchDateFrom);
	
	//Keep a copy of the current query for export purposes.
	g_free(inventory->exportQueryString);
	inventory->exportQueryString = g_strconcat(query_string, NULL);
	
	//If the connection is successful and the query returns a result then the next step is to display those results:
	partsResult = mysql_store_result(partsConnection);
	num_fields = mysql_num_fields(partsResult);

	while ((partsRow = mysql_fetch_row(partsResult))) {
		gchar *id, *barcode, *description, *cost, *price, *dateSold, *soldTo, *invoiceNo;
		float costFloat = 0.00;
		float profitFloat = 0.00;
		float priceFloat = 0.00;
		float marginFloat = 0.00;

		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				id = g_strconcat(partsRow[i], NULL);
			
			if(i == 1)
				barcode = g_strconcat(partsRow[i], NULL);
			
			if(i == 2)
				description = g_strconcat(partsRow[i], NULL);
				
			if(i == 3) {
				cost = g_strdup_printf("%.2f", atof(partsRow[i]));
				costFloat = atof(partsRow[i]);
				//cost = g_strconcat(partsRow[i], NULL);
			}
			if(i == 4) {
				price = g_strdup_printf("%.2f", atof(partsRow[i]));
				priceFloat = atof(partsRow[i]);
				//price = g_strconcat(partsRow[i], NULL);
			}
			if(i == 5)
				dateSold = g_strconcat(partsRow[i], NULL);
				
			if(i == 6)
				soldTo = g_strconcat(partsRow[i], NULL);
				
			if(i == 7)
				invoiceNo = g_strdup(partsRow[i]);				
				
				//g_print("%i\n", i);
		}
		
		gchar *profitChar;
		gchar *marginChar;
		
		profitChar = g_strdup_printf("%.2f", atof(price) - atof(cost));	
		profitFloat = priceFloat - costFloat;	
		
		marginFloat = ((priceFloat - costFloat) / priceFloat) * 100;
		marginChar = g_strdup_printf("%.2f%%", marginFloat);
//*********************************************************************
		/* Processing The Dates */
		GDate *dateTemp, *dateInvoiced, *dateToday;
		int yearScan, monthScan, dayScan;

		dateTemp = g_date_new();
		dateInvoiced = g_date_new();
		dateToday = g_date_new();
		
		g_date_set_time_t(dateToday, time(NULL)); // Get today's date.
				
		sscanf(dateSold, "%d-%d-%d", &yearScan, &monthScan, &dayScan);
		gchar *dateStrTemp = g_strdup_printf("%d-%d-%d", yearScan, monthScan, dayScan);

		g_date_set_parse(dateInvoiced, dateStrTemp);
		g_date_set_parse(dateTemp, dateStrTemp);
		//g_date_add_days(dateTemp, -30);
		
		gboolean validInvoiceDate = FALSE;
		
		if(g_date_compare(dateInvoiced, dateFrom) >=0) {
			if(g_date_compare(dateInvoiced, dateTo) <= 0)
				validInvoiceDate = TRUE;
			else
				validInvoiceDate = FALSE;
		}
		else
			validInvoiceDate = FALSE;		
//************************************************************************		

		/* Now search the tree store to see if the item already exists in it, we search via the id column for identification. This prevents duplicates from getting displayed */
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
			if(foundItem == FALSE && validInvoiceDate == TRUE) {
				gtk_list_store_append (store, &iter);
					
				gtk_list_store_set (store, &iter, ID, id, BARCODE, barcode, DESCRIPTION, description, COST, cost, PROFIT, profitChar, MARGIN, marginChar, PRICE, price, SOLDTO, soldTo, LAST_SOLD, dateSold, INVOICENO, invoiceNo, COSTF, costFloat, PROFITF, profitFloat, MARGINF, marginFloat, PRICEF, priceFloat, -1);	
			}			
		}
		else {
			/* This stores all inventory into the tree */
			if(validInvoiceDate == TRUE) {
				gtk_list_store_append (store, &iter);
			
				gtk_list_store_set (store, &iter, ID, id, BARCODE, barcode, DESCRIPTION, description, COST, cost, PROFIT, profitChar, MARGIN, marginChar, PRICE, price, SOLDTO, soldTo, LAST_SOLD, dateSold, INVOICENO, invoiceNo, COSTF, costFloat, PROFITF, profitFloat, MARGINF, marginFloat, PRICEF, priceFloat, -1);	
			}
		}
		
		g_free(id);
		g_free(barcode);
		g_free(description);
		g_free(cost);
		g_free(profitChar);
		g_free(marginChar);
		g_free(price);
		g_free(dateSold);
		g_free(soldTo);
		g_free(invoiceNo);
		
		g_date_free(dateTemp), g_date_free(dateToday), g_date_free(dateInvoiced), g_free(dateStrTemp);
	}	

	g_free(query_string);	
	mysql_free_result(partsResult);
	
	//***************************************Prev year comparo*********************
	int dateCheckFrom = dateFrom->year - 1;
	int dateCheckTo = dateTo->year - 1;
	
	gchar *numberOfPartsCharPrev = NULL;
	gchar *partCostsCharPrev = NULL;
	gchar *partSalesCharPrev = NULL;
	gchar *partProfitsCharPrev = NULL;
	gchar *partMarginCharPrev = NULL;
	
	// Only run the date check comparison data on years 2011+ since thats when data recording started.
	if(dateCheckFrom > 2010 && dateCheckTo > 2010) {
		// Search for the user entered query
		if(searchString != NULL) {
			if(searchNumMin != NULL && searchNumMax != NULL) {
				if(costPull == TRUE)
					query_string = g_strconcat("SELECT DISTINCT(id), cost, price FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateCompFrom, "' AND dateSold < '", searchDateCompTo, "' AND cost >= ", searchNumMin, " AND cost <= ", searchNumMax, " AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);				
				else if(pricePull == TRUE)
					query_string = g_strconcat("SELECT DISTINCT(id), cost, price FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateCompFrom, "' AND dateSold < '", searchDateCompTo, "' AND price >= ", searchNumMin, " AND price <= ", searchNumMax, " AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);			
				else
					query_string = g_strconcat("SELECT DISTINCT(id), cost, price FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateCompFrom, "' AND dateSold < '", searchDateCompTo, "' AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);
			} 
			else
				query_string = g_strconcat("SELECT DISTINCT(id), cost, price  FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateCompFrom, "' AND dateSold < '", searchDateTo, "' AND ", searchWhere, " LIKE '%", searchString, "%'", " ORDER BY dateSold DESC", NULL);
		}
		else if(searchNumMin != NULL && searchNumMax != NULL)
			query_string = g_strconcat("SELECT DISTINCT(id), cost, price FROM `", SOLD_TABLES, "`", " WHERE dateSold > '", searchDateCompFrom, "' AND dateSold < '", searchDateCompTo, "' AND ", searchWhere, " >= ", searchNumMin, " AND ", searchWhere, " <= ", searchNumMax, " ORDER BY dateSold DESC", NULL);
		else
			query_string = g_strconcat("SELECT DISTINCT(id), cost, price FROM `", SOLD_TABLES, "` WHERE dateSold > '", searchDateCompFrom, "' AND dateSold < '", searchDateCompTo, "' ORDER BY dateSold DESC", NULL);	

		mysql_query(partsConnection, query_string);
		
		// If the connection is successful and the query returns a result then the next step is to display those results:
		partsResult = mysql_store_result(partsConnection);
		num_fields = mysql_num_fields(partsResult);
			
		g_free(searchDateCompTo);
		g_free(searchDateCompFrom);	
		
		float totalCostFloatComp = 0.00;
		float totalPriceFloatComp = 0.00;
		int iComp = 0;
		while ((partsRow = mysql_fetch_row(partsResult))) {
			float costFloatComp = 0.00;
			float priceFloatComp = 0.00;

			for(i = 0; i < num_fields; i++) {
				if(i == 1)
					costFloatComp = atof(partsRow[i]);
				
				if(i == 2)
					priceFloatComp = atof(partsRow[i]);
			}	
			
			totalCostFloatComp = totalCostFloatComp + costFloatComp;
			totalPriceFloatComp = totalPriceFloatComp + priceFloatComp;
			
			iComp++;
		}	
		
		float profitFloatComp = 0.00;
		float marginFloatComp = 0.00;
		
		profitFloatComp = totalPriceFloatComp - totalCostFloatComp;
		marginFloatComp = ((totalPriceFloatComp - totalCostFloatComp) / totalPriceFloatComp) * 100;
		
		inventory->numberOfPartsPrev = iComp;
		inventory->partCostsPrev = totalCostFloatComp;
		inventory->partSalesPrev = totalPriceFloatComp;
		inventory->partProfitsPrev = profitFloatComp;
		inventory->partMarginPrev = marginFloatComp;
		
		g_free(query_string);	
		mysql_free_result(partsResult);
		
		query_string = g_strdup_printf("%i", iComp);
		numberOfPartsCharPrev = g_strconcat(query_string, NULL);
		g_free(query_string);
		
		query_string = g_strdup_printf("%'0.2f", totalCostFloatComp);
		partCostsCharPrev = g_strconcat(query_string, NULL);
		g_free(query_string);
		
		query_string = g_strdup_printf("%'0.2f", totalPriceFloatComp);
		partSalesCharPrev = g_strconcat(query_string, NULL);
		g_free(query_string);
		
		query_string = g_strdup_printf("%'0.2f", profitFloatComp);
		partProfitsCharPrev = g_strconcat(query_string, NULL);
		g_free(query_string);
		
		query_string = g_strdup_printf("%'0.2f%%", marginFloatComp);
		partMarginCharPrev = g_strconcat(query_string, NULL);
		g_free(query_string);		
	}
	else {
		numberOfPartsCharPrev = g_strconcat("-", NULL);
		partCostsCharPrev = g_strconcat("-", NULL);
		partSalesCharPrev = g_strconcat("-", NULL);
		partProfitsCharPrev = g_strconcat("-", NULL);
		partMarginCharPrev = g_strconcat("-", NULL);
		
		inventory->numberOfPartsPrev = 0;
		inventory->partCostsPrev = 0.00;
		inventory->partSalesPrev = 0.00;
		inventory->partProfitsPrev = 0.00;	
		inventory->partMarginPrev = 0.00;		
	}
	
	gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabelPrev), numberOfPartsCharPrev);
	gtk_label_set_text(GTK_LABEL(inventory->partCostsLabelPrev), partCostsCharPrev);
	gtk_label_set_text(GTK_LABEL(inventory->partSalesLabelPrev), partSalesCharPrev);
	gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabelPrev), partProfitsCharPrev);
	gtk_label_set_text(GTK_LABEL(inventory->partMarginLabelPrev), partMarginCharPrev);	
		
	g_free(numberOfPartsCharPrev), g_free(partCostsCharPrev), g_free(partSalesCharPrev), g_free(partProfitsCharPrev), g_free(partMarginCharPrev);		
	//***************************************End of prev year comparo*********************
	
	mysql_close(partsConnection);

	return 0;
}

/* Gets data about a item before removing it from the part sales and returning it to stock */
static int getItemData(intrackInventory *inventory, removeSale *oldData, gchar *partNo) {

	gchar *query_string;
	int i;
	int num_fields;	
		
	int query_state;	

	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strconcat(inventory->mysqlDatabase, NULL);
	query_state = mysql_select_db(connection, query_string);

	//g_print("%s\n", query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}
	
	g_free(query_string);
	query_string = g_strconcat("SELECT id, partNo, cost, costAvg, stock, price FROM `", INVENTORY_TABLES, "`", " WHERE partNo='", partNo, "'", NULL);

	//g_print("%s\n", query_string);

	query_state=mysql_query(connection, query_string);
	g_free(query_string);
	
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		mysql_close(connection);

		return 1;
	}
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	int i2 = 0;
	gchar *cost, *avgCost, *stock, *price;
	
	while ((row = mysql_fetch_row(result))) {
		for(i = 0; i < num_fields; i++) {
			//g_print("%s\n", row[i]);
			
			if(i == 2)
				cost = g_strdup(row[i]);
				
			if(i == 3)
				avgCost = g_strdup(row[i]);
				
			if(i == 4)
				stock = g_strdup(row[i]);
				
			if(i == 5)
				price = g_strdup(row[i]);				
		}
		i2++;
	}	
	
	oldData->oldStockData = g_strdup(stock);
	oldData->costData = g_strdup(cost);
	oldData->oldAvgCostData = g_strdup(avgCost);
	oldData->oldPrice = g_strdup(price);
	//g_print("Cost Data Org: %s\n", oldData->costData);

	g_free(stock), g_free(avgCost), g_free(cost), g_free(price);
	
	if(i2 > 1)
		g_print("multiple results of part number in inventory, could be data errors\n");
	
	//g_print("Cost Data: %s\n", costData);
		
	mysql_free_result(result); // Free up some memory.
	mysql_close(connection);			
	
	return 0;
}

/* Remove a item from the inventory database */
static void beginItemRemoval(GtkTreeRowReference *ref, intrackInventory *inventory) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData, *partNo;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* id column */
    gtk_tree_model_get(model, &iter, BARCODE, &partNo, -1); /* part no column */
	
    if(!checkIfExist(partNo)) {
		removeSale *oldData;
		oldData = (removeSale*) g_malloc (sizeof (removeSale)); // Allocate memory for the data.
			
		oldData->oldStockData = g_strdup(NULL);
		oldData->costData = g_strdup(NULL);
		oldData->oldAvgCostData = g_strdup(NULL);
		oldData->oldPrice = g_strdup(NULL);
		
		getItemData(inventory, oldData, partNo);
			/*
				g_print("oldStockData=%s\n", oldData->oldStockData);
				g_print("costData=%s\n", oldData->costData);
				g_print("oldAvgCostData=%s\n", oldData->oldAvgCostData);
				g_print("partNo=%s\n", partNo);
				g_print("oldPrice=%s\n", oldData->oldPrice);
			*/
		gchar *newStockChar, *newAvgCostChar, *profitChar, *priceChar;
		int newStock, oldStock, stockDiff;
		float oldAvgCost, newAvgCost, cost;

		//oldAvgCost = getAverageCost(whereData);
		oldAvgCost = atof(oldData->oldAvgCostData);

		newStock = atoi(oldData->oldStockData) + 1;
		newStockChar = g_strdup_printf("%i", newStock);

		oldStock = atoi(oldData->oldStockData);
		cost = atof(oldData->costData);
		
		/* When stock is added */
		if(newStock > oldStock) {
			stockDiff = newStock - oldStock;
			newAvgCost = ((oldAvgCost * oldStock) + (cost * stockDiff)) / (oldStock + stockDiff);
		}
		/* When stock is subtracted */
		else if((newStock < oldStock) && newStock > 0) {
			stockDiff = abs(oldStock - newStock);
			newAvgCost = ((oldAvgCost * oldStock) + ((oldAvgCost * stockDiff) * -1)) / (oldStock - stockDiff);
		}
		else { /* When stock is now 0 */
			stockDiff = 0;
			newAvgCost = cost;
		}
		
		newAvgCostChar = g_strdup_printf("%f", newAvgCost);
				
		/* Update the database */
		databaseReturnStockItem(inventory->mysqlDatabase, inventory->inventoryTable, partNo, newStockChar, "stock");
		//databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, partNo, newAvgCostChar, "costAvg"); /* Update the new average cost */
		
		g_free(newAvgCostChar);
		newAvgCostChar = g_strdup_printf("%0.2f", newAvgCost);

		g_free(newStockChar);
		g_free(newAvgCostChar);
		
		/* Update the database */
		databaseRemoveItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData);
		
		g_free(oldData->oldStockData), g_free(oldData->costData), g_free(oldData->oldAvgCostData), g_free(oldData->oldPrice);		
		g_free(oldData);
		
		/* Remove the row from the tree */
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);		

	}
	else {
		printMessage(ERROR_RETURN_STOCK, inventory->mainWindow);
	}

	g_free(whereData), g_free(partNo);
}

/* Prepares the remove process of a item from the inventory */
static void prepareItemRemoval(GtkWidget *widget, intrackInventory *inventory) {

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	/* Create tree row references to all of the selected rows */
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	/* Remove each of the selected rows pointed to by the row reference */
	g_list_foreach(references, (GFunc) beginItemRemoval, inventory);
	
	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
}

static void calculateTreeTotals(GtkListStore *store, intrackInventory *inventory) {
	GtkTreeIter iter;
	gchar *rowtext, **split, *join;
	//gchar *coststext, **splitcosts, *joincosts;
	//gchar *profitstext, **splitprofits, *joinprofits;
	int numberOfParts = 0;
	float partCosts = 0;
	float partSales = 0;
	float partProfits = 0;
	float partMargin = 0;

	// Calculate the total sales, and the total sales amount in the latest tree starting from the very first row in the tree
	if(gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter)) {
		// Number Of Parts Sold
		numberOfParts = numberOfParts + 1;
		
		// Parts Costs
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, COST, &rowtext, -1);
		partCosts = partCosts + atof(rowtext);
		g_free(rowtext);
		
		// Parts Costs Average
		gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, PRICE, &rowtext, -1);
		partSales = partSales + atof(rowtext);
		g_free(rowtext);
		
		while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter)) {
			// Parts Costs
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, COST, &rowtext, -1);
			partCosts = partCosts + atof(rowtext);
			g_free(rowtext);
		
			// Parts Costs Average
			gtk_tree_model_get(GTK_TREE_MODEL (store), &iter, PRICE, &rowtext, -1);
			partSales = partSales + atof(rowtext);
			g_free(rowtext);
			
			// Number Of Parts
			numberOfParts++;
		}
	}

	partProfits = partSales - partCosts;
	partMargin = ((partSales - partCosts) / partSales) * 100;
	
    char *locale;
    locale = setlocale(LC_NUMERIC, "en_US.iso88591");
    //printf("%'d\n", 12345);
    setlocale(LC_NUMERIC, locale);
    
	gchar *numberOfPartsChar = NULL;
	gchar *partCostsChar = NULL;
	gchar *partSalesChar = NULL;
	gchar *partProfitsChar = NULL;
	gchar *partMarginChar = NULL;
	
	numberOfPartsChar = g_strdup_printf("%i", numberOfParts);
	partCostsChar = g_strdup_printf("%'0.2f", partCosts);
	partSalesChar = g_strdup_printf("%'0.2f", partSales);
	partProfitsChar = g_strdup_printf("%'0.2f", partProfits);
	partMarginChar = g_strdup_printf("%'0.2f%%", partMargin);

	float num_parts_diff = 0.00;
	float num_parts_perc = 0.00;
	
	float diff_costs = 0;
	float perc_costs = 0;
	
	float diff_sales = 0;
	float perc_sales = 0;
	
	float diff_profit = 0;
	float perc_profit = 0;
	
	float diff_margin = 0;
	float perc_margin = 0;
	
	if(inventory->numberOfPartsPrev > 0) {
		gchar *diff_num_parts_char = NULL;
		gchar *perc_num_parts_char = NULL;

		gchar *diff_costs_char = NULL;
		gchar *perc_costs_char = NULL;

		gchar *diff_sales_char = NULL;
		gchar *perc_sales_char = NULL;

		gchar *diff_profit_char = NULL;
		gchar *perc_profit_char = NULL;
		
		gchar *diff_margin_char = NULL;
		gchar *perc_margin_char = NULL;

		float lowInt = 0.00;
		float highInt = 0.00;
		
		// Number of items
		if(inventory->numberOfPartsPrev > numberOfParts) {
			lowInt = numberOfParts;
			highInt = inventory->numberOfPartsPrev;
			
			num_parts_diff = 100 - (lowInt/highInt * 100);
			num_parts_perc = ((highInt-lowInt) / lowInt) * 100;
			
			diff_num_parts_char = g_strdup_printf("-%.2f%%", num_parts_diff);
			perc_num_parts_char = g_strdup_printf("-%.2f%%", num_parts_perc);
		}
		else if(inventory->numberOfPartsPrev < numberOfParts) {
			lowInt = inventory->numberOfPartsPrev;
			highInt = numberOfParts;
			
			num_parts_diff = 100 - (lowInt/highInt * 100);
			num_parts_perc = ((highInt-lowInt) / lowInt) * 100;
			diff_num_parts_char = g_strdup_printf("+%.2f%%", num_parts_diff);
			perc_num_parts_char = g_strdup_printf("+%.2f%%", num_parts_perc);
		}
		else {
			num_parts_diff = 0;
			num_parts_perc = 0;
			diff_num_parts_char = g_strdup_printf("%.2f%%", num_parts_diff);
			perc_num_parts_char = g_strdup_printf("%.2f%%", num_parts_perc);
		}
		
		// Costs
		if(inventory->partCostsPrev > partCosts) {
			diff_costs = 100 - (partCosts/inventory->partCostsPrev * 100);
			perc_costs = ((inventory->partCostsPrev-partCosts) / partCosts) * 100;
			
			diff_costs_char = g_strdup_printf("-%.2f%%", diff_costs);
			perc_costs_char = g_strdup_printf("-%.2f%%", perc_costs);
		}
		else if(inventory->partCostsPrev < partCosts) {
			diff_costs = 100 - (inventory->partCostsPrev/partCosts * 100);
			perc_costs = ((partCosts-inventory->partCostsPrev) / inventory->partCostsPrev) * 100;

			diff_costs_char = g_strdup_printf("+%.2f%%", diff_costs);
			perc_costs_char = g_strdup_printf("+%.2f%%", perc_costs);
		}
		else {
			diff_costs = 0;
			perc_costs = 0;
			
			diff_costs_char = g_strdup_printf("%.2f%%", diff_costs);
			perc_costs_char = g_strdup_printf("%.2f%%", perc_costs);
		}
		
		// Sales
		if(inventory->partSalesPrev > partSales) {
			diff_sales = 100 - (partSales/inventory->partSalesPrev * 100);
			perc_sales = ((inventory->partSalesPrev-partSales) / partSales) * 100;
			
			diff_sales_char = g_strdup_printf("-%.2f%%", diff_sales);
			perc_sales_char = g_strdup_printf("-%.2f%%", perc_sales);
		}
		else if(inventory->partSalesPrev < partSales) {
			diff_sales = 100 - (inventory->partSalesPrev/partSales * 100);
			perc_sales = ((partSales-inventory->partSalesPrev) / inventory->partSalesPrev) * 100;
			
			diff_sales_char = g_strdup_printf("+%.2f%%", diff_sales);
			perc_sales_char = g_strdup_printf("+%.2f%%", perc_sales);
		}
		else {
			diff_sales = 0;
			perc_sales = 0;
			
			diff_sales_char = g_strdup_printf("%.2f%%", diff_sales);
			perc_sales_char = g_strdup_printf("%.2f%%", perc_sales);
		}
		
		// Profits
		if(inventory->partProfitsPrev > partProfits) {
			diff_profit = 100 - (partProfits/inventory->partProfitsPrev * 100);
			perc_profit = ((inventory->partProfitsPrev-partProfits) / partProfits) * 100;
			
			diff_profit_char = g_strdup_printf("-%.2f%%", diff_profit);
			perc_profit_char = g_strdup_printf("-%.2f%%", perc_profit);
		}
		else if(inventory->partProfitsPrev < partProfits) {
			diff_profit = 100 - (inventory->partProfitsPrev/partProfits * 100);
			perc_profit = ((partProfits-inventory->partProfitsPrev) / inventory->partProfitsPrev) * 100;
			
			diff_profit_char = g_strdup_printf("+%.2f%%", diff_profit);
			perc_profit_char = g_strdup_printf("+%.2f%%", perc_profit);
		}
		else {
			diff_profit = 0;
			perc_profit = 0;
			
			diff_profit_char = g_strdup_printf("%.2f%%", diff_profit);
			perc_profit_char = g_strdup_printf("%.2f%%", perc_profit);
		}

		// Margin
		if(inventory->partMarginPrev > partMargin) {
			diff_margin = 100 - (partMargin/inventory->partMarginPrev * 100);
			perc_margin = ((inventory->partMarginPrev-partMargin) / partMargin) * 100;
			
			diff_margin_char = g_strdup_printf("-%.2f%%", diff_margin);
			perc_margin_char = g_strdup_printf("-%.2f%%", perc_margin);
		}
		else if(inventory->partMarginPrev < partMargin) {
			diff_margin = 100 - (inventory->partMarginPrev/partMargin * 100);
			perc_margin = ((partMargin-inventory->partMarginPrev) / inventory->partMarginPrev) * 100;
			
			diff_margin_char = g_strdup_printf("+%.2f%%", diff_margin);
			perc_margin_char = g_strdup_printf("+%.2f%%", perc_margin);
		}
		else {
			diff_margin = 0;
			perc_margin = 0;
			
			diff_margin_char = g_strdup_printf("%.2f%%", diff_margin);
			perc_margin_char = g_strdup_printf("%.2f%%", perc_margin);
		}		
		
		gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabelDiff), diff_num_parts_char);
		gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabelPerc), perc_num_parts_char);

		gtk_label_set_text(GTK_LABEL(inventory->partCostsLabelDiff), diff_costs_char);
		gtk_label_set_text(GTK_LABEL(inventory->partCostsLabelPerc), perc_costs_char);

		gtk_label_set_text(GTK_LABEL(inventory->partSalesLabelDiff), diff_sales_char);
		gtk_label_set_text(GTK_LABEL(inventory->partSalesLabelPerc), perc_sales_char);

		gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabelDiff), diff_profit_char);
		gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabelPerc), perc_profit_char);
		
		gtk_label_set_text(GTK_LABEL(inventory->partMarginLabelDiff), diff_margin_char);
		gtk_label_set_text(GTK_LABEL(inventory->partMarginLabelPerc), perc_margin_char);		
		
		g_free(diff_num_parts_char), g_free(diff_costs_char), g_free(diff_sales_char), g_free(diff_profit_char), g_free(diff_margin_char);
		g_free(perc_num_parts_char), g_free(perc_costs_char), g_free(perc_sales_char), g_free(perc_profit_char), g_free(perc_margin_char);
	}
	else {
		
		gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabelDiff), "-");
		gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabelPerc), "-");
		
		gtk_label_set_text(GTK_LABEL(inventory->partCostsLabelDiff), "-");
		gtk_label_set_text(GTK_LABEL(inventory->partCostsLabelPerc), "-");
		
		gtk_label_set_text(GTK_LABEL(inventory->partSalesLabelDiff), "-");
		gtk_label_set_text(GTK_LABEL(inventory->partSalesLabelPerc), "-");
		
		gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabelDiff), "-");
		gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabelPerc), "-");
		
		gtk_label_set_text(GTK_LABEL(inventory->partMarginLabelDiff), "-");
		gtk_label_set_text(GTK_LABEL(inventory->partMarginLabelPerc), "-");
		
	}

	
	gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabel), numberOfPartsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partCostsLabel), partCostsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partSalesLabel), partSalesChar);
	gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabel), partProfitsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partMarginLabel), partMarginChar);	
	
	
	g_free(numberOfPartsChar), g_free(partCostsChar), g_free(partSalesChar), g_free(partProfitsChar), g_free(partMarginChar);
}

// Calculate the selection of items totals
static void beginCalculate(GtkTreeRowReference *ref, intrackInventory *inventory) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *costData, *priceData;
	gchar *idData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &idData, -1); /* id */
    gtk_tree_model_get(model, &iter, COST, &costData, -1); /* cost */
    gtk_tree_model_get(model, &iter, PRICE, &priceData, -1); /* price */
    
    //g_print("%i\n", atoi(idData));
    
	inventory->numberOfParts++;
	inventory->partCosts = inventory->partCosts + atof(costData);
	inventory->partSales = inventory->partSales + atof(priceData);
	
	g_free(costData), g_free(priceData);
	g_free(idData);
}

/* Calculates the selection of items */
static void prepareCalculate(GtkWidget *widget, intrackInventory *inventory) {
	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
	ptr = rows;
	
	/* Create tree row references to all of the selected rows */
	while(ptr != NULL) {
		ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
		references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
		gtk_tree_row_reference_free(ref);
		ptr = ptr->next;
	}

	/* Reset the selection totals */
	inventory->numberOfParts = 0;
	inventory->partCosts = 0.00;
	inventory->partSales = 0.00;
	inventory->partProfits = 0.00;
	inventory->partMargin = 0.00;

	/* Process each of the selected rows pointed to by the row reference */
	g_list_foreach(references, (GFunc) beginCalculate, inventory);
	
	inventory->partProfits = inventory->partSales - inventory->partCosts;
	inventory->partMargin = ((inventory->partSales - inventory->partCosts) / inventory->partSales) * 100;

	//g_print("Number of Parts: %i\n", inventory->numberOfParts);
	//g_print("Part Costs: %.2f\n", inventory->partCosts);
	//g_print("Part Sales: %.2f\n", inventory->partSales);
	//g_print("Part Profits: %.2f\n", inventory->partProfits);

    //char *locale;
    //locale = setlocale(LC_NUMERIC, "en_US.iso88591");
    //setlocale(LC_NUMERIC, locale);
        
	gchar *numberOfPartsChar = NULL;
	gchar *partCostsChar = NULL;
	gchar *partSalesChar = NULL;
	gchar *partProfitsChar = NULL;
	gchar *partMarginChar = NULL;
	
	numberOfPartsChar = g_strdup_printf("%i", inventory->numberOfParts);
	partCostsChar = g_strdup_printf("%'0.2f", inventory->partCosts);
	partSalesChar = g_strdup_printf("%'0.2f", inventory->partSales);
	partProfitsChar = g_strdup_printf("%'0.2f", inventory->partProfits);
	
	if(inventory->partMargin > 0.00)
		partMarginChar = g_strdup_printf("%'0.2f%%", inventory->partMargin);
	else
		partMarginChar = g_strdup_printf("-", inventory->partMargin);

	gtk_label_set_text(GTK_LABEL(inventory->numberOfPartsLabel1), numberOfPartsChar);	
	gtk_label_set_text(GTK_LABEL(inventory->partCostsLabel1), partCostsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partSalesLabel1), partSalesChar);
	gtk_label_set_text(GTK_LABEL(inventory->partProfitsLabel1), partProfitsChar);
	gtk_label_set_text(GTK_LABEL(inventory->partMarginLabel1), partMarginChar);

	g_free(numberOfPartsChar), g_free(partCostsChar), g_free(partSalesChar), g_free(partProfitsChar), g_free(partMarginChar);	
	
	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
}

/* Prepares the remove process of a item from the inventory */
/* NOT USED, DELETE THIS FUNCTION */
static void prepareItemRemovalOld(GtkWidget *widget, intrackInventory *inventory) {
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    GtkTreePath 	*path;
    GtkTreeSelection *selection;

	gchar *whereData;
	
    /* Need to pull the current selected row from the treeview */
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(inventory->inventoryTree));

	/* Now need to pull the path and iter from the selected row */
	if(gtk_tree_selection_get_selected(selection, &model, &iter)) {
		path = gtk_tree_model_get_path(model, &iter);

		//model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
		//gtk_tree_model_get_iter_from_string(model, &iter, path);
		gtk_tree_model_get(model, &iter, BARCODE, &whereData, -1); /* barcode column */	

		/* Update the database */
		databaseRemoveItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData);

		/* Write code here to update the barcode in taxGroupItems database */
		
		/* Remove the row from the tree */
		gtk_list_store_remove(GTK_LIST_STORE(model), &iter);

		g_free(whereData);
	}
	else
		printMessage("ERROR: Select a item to remove from the inventory database", inventory->mainWindow);
}

/* Edits the cost */
static int cellClickedCost(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	//if(checkInput(newText))
		//return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *price, *whereData, *lastSold;
	gchar *newCostChar;
	gchar *profitChar;
	gchar *marginChar;
	float newCost, profit, margin;
	//float newPrice;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* id column */	
   	gtk_tree_model_get(model, &iter, PRICE, &price, -1);
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	

	newCost = atof(newText);
	newCostChar = g_strdup_printf("%.2f", newCost);
		
	/* Update the database */
	//databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newPriceChar, "price");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newCostChar, "cost");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, lastSold, "dateSold"); // Need to reset update since database set to NOW() during updates.
		
	//profit = newPrice - atof(costAvgChar);
	profit = atof(price) - atof(newCostChar);
	profitChar = g_strdup_printf("%.2f", profit);

	margin = ((atof(price) - newCost) / atof(price)) * 100;
	marginChar = g_strdup_printf("%.2f%%", margin);
	
	/* Update the tree cell with the new data */
	//gtk_list_store_set(GTK_LIST_STORE(model), &iter, PRICE, newPriceChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COST, newCostChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, COSTF, newCost, -1);
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, PROFIT, profitChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, PROFITF, profit, -1);
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, MARGIN, marginChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, MARGINF, margin, -1);	
		
	g_free(newCostChar), g_free(profitChar), g_free(marginChar);
		
	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(price), g_free(lastSold);
	
	return 0;
}

/* Edits the price */
static int cellClickedPrice(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	//if(checkInput(newText))
		//return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *lastSold;
	gchar *newPriceChar;
	gchar *costAvgChar, *profitChar, *marginChar;
	float costAvg, profit, margin;
	float newPrice;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* id column */	
   	gtk_tree_model_get(model, &iter, COST, &costAvgChar, -1);
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	

	newPrice = atof(newText);
	newPriceChar = g_strdup_printf("%.2f", newPrice);
		
	/* Update the database */
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newPriceChar, "price");
	//databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, costAvgChar, "cost");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, lastSold, "dateSold"); // Need to reset update since database set to NOW() during updates.
		
	profit = newPrice - atof(costAvgChar);
	profitChar = g_strdup_printf("%.2f", profit);

	margin = ((newPrice - atof(costAvgChar)) / newPrice) * 100;
	marginChar = g_strdup_printf("%.2f%%", margin);
	
	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, PRICE, newPriceChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, PRICEF, newPrice, -1);
	
	//gtk_list_store_set(GTK_LIST_STORE(model), &iter, COST, costAvgChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, PROFIT, profitChar, -1);
	
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, MARGIN, marginChar, -1);
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, MARGINF, margin, -1);		
		
	g_free(newPriceChar), g_free(costAvgChar), g_free(profitChar), g_free(marginChar);
		
	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(lastSold);
	
	return 0;
}

/* Edits the sold to */
static int cellClickedSoldTo(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *lastSold;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* ID column */	
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	
    
	/* Update the database */
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "soldTo");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, lastSold, "dateSold"); // Need to reset update since database set to NOW() during updates.

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, SOLDTO, newText, -1);

	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(lastSold);
	
	return 0;
}

/* Edits the description */
static int cellClickedDescription(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *lastSold;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* ID column */	
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	
    
	/* Update the database */
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "partDesc");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, lastSold, "dateSold"); // Need to reset update since database set to NOW() during updates.

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, DESCRIPTION, newText, -1);

	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(lastSold);
	
	return 0;
}

static int check_date(gchar *newText) {  
	int l,d,y,m,logic = 0;
	gchar *dd, *mm, *yy;

			GDate *dateTemp, *dateToday;
			dateTemp = g_date_new();
			dateToday = g_date_new();
			
			g_date_set_time_t(dateToday, time(NULL)); // Get today's date.
						
	//int y, m, d;
	sscanf(newText, "%d-%d-%d", &y, &m, &d);
		gchar *dateStrTemp = g_strdup_printf("%d-%d-%d", y, m, d);
		g_date_set_parse(dateTemp, dateStrTemp);		
	yy = g_strdup_printf("%i", y);
	mm = g_strdup_printf("%i", m);
	dd = g_strdup_printf("%i", d);

   // Case of months having 31 days
   if(m == 1 || m == 3 || m == 5 || m == 7 || m == 8 || m == 10 || m == 12)
   {  if( d>=1 && d<=31)
      { logic = 1;
      }
   }
   // Case of Februaury
   else if(m == 2)
   {   if(d>=1 && d<=28)
       { logic =1;
       }
       if(d == 29 && y % 4 == 0)
       { logic=1;
       }
   }
   // Case of months having 30 days
   else if(m == 4 || m == 6 || m == 9 || m == 11)
   {
       if(d>=1 && d<=30)
       {  logic =1;
       }
   }
	
	if(logic == 1) {
		if(g_date_compare(dateTemp, dateToday) > 0) {
			logic = 2;
		}
		
		if(g_date_valid(dateTemp) == FALSE)
			logic = 0;
	}
		
	g_free(mm), g_free(dd), g_free(yy);
	g_free(dateStrTemp);
	g_date_free(dateTemp), g_date_free(dateToday);
	
   return logic;
}

/* Edits the description */
static int cellClickedDateSold(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
	
	int c;
	c=check_date(newText);
	
    if(c==0) {
		//g_print("date invalid\n");
		gchar *errorMessage = g_strdup_printf("invalid date!");
		printMessage(errorMessage, inventory->mainWindow);
		g_free(errorMessage);
		
		return 0;	
	}
	//else if(c == 2)
		//g_print("date in future\n");
	//else
		//g_print("date valid\n");
	
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *lastSold;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* ID column */	
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	
    
    int yy, mm, dd, h, m, s;
    int yearNew, monthNew, dayNew;
	sscanf(newText, "%d-%d-%d", &yearNew, &monthNew, &dayNew);
	sscanf(lastSold, "%d-%d-%d %d:%d:%d", &yy, &mm, &dd, &h, &m, &s);
	
	gchar *hChar, *mChar, *sChar;
	gchar *mmChar, *ddChar;
	
	if(h < 10)
		hChar = g_strdup_printf("0%i", h);
	else
		hChar = g_strdup_printf("%i", h);
		
	if(m < 10)
		mChar = g_strdup_printf("0%i", m);
	else
		mChar = g_strdup_printf("%i", m);
	
	if(s < 10)
		sChar = g_strdup_printf("0%i", s);
	else
		sChar = g_strdup_printf("%i", s);
		
	if(monthNew < 10)
		mmChar = g_strdup_printf("0%i", monthNew);
	else
		mmChar = g_strdup_printf("%i", monthNew);
		
	if(dayNew < 10)
		ddChar = g_strdup_printf("0%i", dayNew);
	else
		ddChar = g_strdup_printf("%i", dayNew);						
		
	gchar *newDate = g_strdup_printf("%i-%s-%s %s:%s:%s", yearNew, mmChar, ddChar, hChar, mChar, sChar);
	//g_print("%s\n", newDate);
    
	/* Update the database */
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newDate, "dateSold");

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, LAST_SOLD, newText, -1);

	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(lastSold), g_free(newDate);
    g_free(hChar), g_free(mChar), g_free(sChar);
    g_free(mmChar), g_free(ddChar);
	
	return 0;
}

/* Edits the part number */
static int cellClickedPartNo(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *lastSold;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* ID column */	
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	
    
	/* Update the database */
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "partNo");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, lastSold, "dateSold"); // Need to reset update since database set to NOW() during updates.

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, BARCODE, newText, -1);

	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(lastSold);
	
	return 0;
}

/* Edits the invoice number */
static int cellClickedInvoice(GtkCellRendererText *render, gchar *path, gchar *newText, intrackInventory *inventory) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData, *lastSold;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* ID column */	
    gtk_tree_model_get(model, &iter, LAST_SOLD, &lastSold, -1); /* date sold column */	
    
	/* Update the database */
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, newText, "invoiceNo");
	databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, whereData, lastSold, "dateSold"); // Need to reset update since database set to NOW() during updates.

	/* Update the tree cell with the new data */
	gtk_list_store_set(GTK_LIST_STORE(model), &iter, INVOICENO, newText, -1);

	calculateTreeTotals(GTK_LIST_STORE(model), inventory);

    g_free(whereData), g_free(lastSold);
	
	return 0;
}

/* Removes a item from the sales database */
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
	query_string = g_strconcat("DELETE FROM `", SOLD_TABLES, "`", " WHERE id='", id, "'", NULL);

	query_state=mysql_query(connection, query_string);
	
	/* Failed to update the data */
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

/* Updates a item in the inventory database. */
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
	
	/* Failed to update the data */
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

/* Updates a item in the inventory database. */
static int databaseEditItem(gchar *mysqlDatabase, gchar *inventoryTable, gchar *id, gchar *newCode, gchar *column) {
	
	gchar *query_string;
	int query_state;	

	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strconcat(mysqlDatabase, NULL);
	query_state = mysql_select_db(connection, query_string);

	//g_print("%s\n", query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}
	
	g_free(query_string);
	query_string = g_strconcat("UPDATE `", SOLD_TABLES, "`", " SET ", column, "='", newCode, "' WHERE id='", id, "'", NULL);

	//g_print("%s\n", query_string);

	query_state=mysql_query(connection, query_string);
	
	/* Failed to update the data */
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

/* Updates a item in the inventory database. */
static int databaseReturnStockItem(gchar *mysqlDatabase, gchar *inventoryTable, gchar *id, gchar *newCode, gchar *column) {
	
	gchar *query_string;
	int query_state;	

	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strconcat(mysqlDatabase, NULL);
	query_state = mysql_select_db(connection, query_string);

	//g_print("%s\n", query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		g_free(query_string);
		mysql_close(connection);
		
		return 1;
	}
	
	g_free(query_string);
	query_string = g_strconcat("UPDATE `", INVENTORY_TABLES, "`", " SET ", column, "='", newCode, "' WHERE partNo='", id, "'", NULL);

	//g_print("%s\n", query_string);

	query_state=mysql_query(connection, query_string);
	
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		g_free(query_string);	
		mysql_close(connection);

		return 1;
	}
	
	// Update total sold
	g_free(query_string);
	query_string = g_strconcat("UPDATE `", INVENTORY_TABLES, "`", " SET totalSold=totalSold - 1 WHERE partNo='", id, "'", NULL);
	//g_print("%s\n", query_string);

	query_state=mysql_query(connection, query_string);
	
	/* Failed to update the data */
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

/* Makes changes to the tax group items */
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


/* Gets the average cost of the item from the database */
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
	costQuery = g_strconcat("SELECT costAvg FROM `", mysqlTables, "` WHERE partNo = '", barcode, "'", NULL);
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

static void cell_data_func(GtkTreeViewColumn *column, GtkCellRenderer *renderer, GtkTreeModel *model, GtkTreeIter *iter, gpointer data) {
	intrackInventory *inventory = (intrackInventory *)data;
	
	prepareCalculate(NULL, inventory);

}

/* Create the tree view and columns */
static void setupInventoryTree(intrackInventory *inventory) { 
	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. ID purposes only.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	gtk_tree_view_column_set_cell_data_func(column, renderer, cell_data_func, (gpointer) inventory, NULL);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Code #", renderer, "text", BARCODE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, BARCODE); 
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	//g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedPartNo), (gpointer) inventory); // Disabled currently because need to check return to stock if compatible.
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Description", renderer, "text", DESCRIPTION, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DESCRIPTION);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDescription), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	//renderer = gtk_cell_renderer_text_new ();
	//column = gtk_tree_view_column_new_with_attributes("Cost", renderer, "text", COST, NULL);
	//gtk_tree_view_column_set_resizable(column, TRUE);
	//gtk_tree_view_column_set_reorderable(column, TRUE);
	//gtk_tree_view_column_set_sort_column_id (column, COST); 
	//gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	GtkAdjustment 	*costAdj;
	costAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00, 100000000.00, 0.01, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", costAdj,
                            "digits", 2, NULL);
	column = gtk_tree_view_column_new_with_attributes("Cost", renderer, "text", COST, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, COSTF);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);		
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCost), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Profit", renderer, "text", PROFIT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PROFITF);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Margin", renderer, "text", MARGIN, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, MARGINF);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);		
	
	GtkAdjustment 	*priceAdj;
	priceAdj = GTK_ADJUSTMENT (gtk_adjustment_new (0.00, 0.00, 100000000.00, 0.01, 5.0, 0.0));
	renderer = gtk_cell_renderer_spin_new();
	g_object_set (renderer, "editable", TRUE,
                            "adjustment", priceAdj,
                            "digits", 2, NULL);
	column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", PRICE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_alignment(column, 1.0);
	g_object_set (renderer, "xalign", 1.0, NULL);		
	gtk_tree_view_column_set_sort_column_id (column, PRICEF);
	//g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedPrice), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);

/*		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", PRICE, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PRICE); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedPrice), (gpointer) inventory);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
*/	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Sold To", renderer, "text", SOLDTO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SOLDTO);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedSoldTo), (gpointer) inventory);	
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Date Sold", renderer, "text", LAST_SOLD, NULL);
	//gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, LAST_SOLD);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDateSold), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Invoice #", renderer, "text", INVOICENO, NULL);
	//gtk_tree_view_column_set_clickable (column, TRUE);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, INVOICENO);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedInvoice), (gpointer) inventory);
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	

	// These 3 cells below are hidden and used for cell order organization only.
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Cost", renderer, "text", COSTF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, COSTF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Profit", renderer, "text", PROFITF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, PROFITF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Margin", renderer, "text", MARGINF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, MARGINF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);		
		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Price", renderer, "text", PRICEF, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. Used for cell order organization only.
	gtk_tree_view_column_set_sort_column_id (column, PRICEF); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (inventory->inventoryTree), column);
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

/* Keyboard key press event on the inventory tree */
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, intrackInventory *inventory) {
	
    switch(ev->keyval)
    {       
        case GDK_Up:
				/* Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection. */
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

	//prepareCalculate(NULL, inventory);

	if(gtk_tree_selection_count_selected_rows(inventory->selection) == 0) {
		gtk_widget_set_sensitive(inventory->returnButton, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
	}
	else if(gtk_tree_selection_count_selected_rows(inventory->selection) > 1) {
		gtk_widget_set_sensitive(inventory->returnButton, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
	}
	else {
		gtk_widget_set_sensitive(inventory->returnButton, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, TRUE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
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

/* Mouse click event on the tree */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackInventory *inventory) {

    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(inventory->inventoryTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
		gtk_widget_set_sensitive(inventory->returnButton, FALSE);
        return FALSE;
	}

	/* Set the sensitivity on the viewItem Button */
	g_timeout_add(1, selectionTimer, (gpointer) inventory); 	

    /* LMB */
    switch(ev->button)
    {
        case 1: /* 1 = left click */
            break;

        case 3: /* 3 = right click */
				//gtk_menu_popup(GTK_MENU(inventory->invenMenu), NULL, NULL, NULL, NULL, 3, ev->time);
            break;
    }
    
	/* Keep track of the selected item */
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
    gchar *rowBarcode;
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &rowBarcode, -1); /* 1 is the id column */
    
    g_free(inventory->selectedItemCode);
    inventory->selectedItemCode = g_strconcat(rowBarcode, NULL);
   	gtk_widget_set_sensitive(inventory->returnButton, TRUE);
    g_free(rowBarcode);
    
    /* free our path */
    gtk_tree_path_free(path);  

    return FALSE;	
}

static gboolean selectionTimer(gpointer data) {
	
	intrackInventory *inventory = (intrackInventory *)data;

	//prepareCalculate(NULL, inventory);

	/* If the user selects multiple rows in the basket tree, then turn off the itemView button */
	if(gtk_tree_selection_count_selected_rows(inventory->selection) > 0) {
		gtk_widget_set_sensitive(inventory->returnButton, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);	
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
	}
	else {
		gtk_widget_set_sensitive(inventory->returnButton, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, TRUE);		
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->editItemButton, TRUE);
	}		
	
	return FALSE;
}

/* Get the selected row from a key press (part #2) */
static void keyPressGetRow(GtkTreeRowReference *ref, intrackInventory *inventory) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *rowBarcode;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(inventory->inventoryTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &rowBarcode, -1); /* 1 is the barcode column */
    
    g_free(inventory->selectedItemCode);
    inventory->selectedItemCode = g_strconcat(rowBarcode, NULL);
    g_free(rowBarcode);
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

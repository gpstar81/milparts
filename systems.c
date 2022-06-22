//      systems.c
//      Copyright 2011 - 2014 Michael Rajotte <michael@michaelrajotte.com>

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
#include "systems.h"

/* Loads up the fees configuration */
void loadSystems(GtkBuilder *builder, GtkWidget *mainWindow) {
	intrackAccounts *accounts;
	accounts = (intrackAccounts*) g_malloc (sizeof (intrackAccounts));
	
	accounts->mainWindow = mainWindow;
	accounts->selectedAccount = g_strdup(NULL);

    /* Exit program button from file / quit. */
    GtkWidget	*exitAccount;
    exitAccount = GTK_WIDGET(gtk_builder_get_object(builder, "accountsCloseButton"));
   	g_signal_connect(G_OBJECT(exitAccount), "activate", G_CALLBACK(hideGtkWidget), mainWindow);	

	/* Close the window */
    GtkWidget	*accountCloseButton;
    accountCloseButton = GTK_WIDGET(gtk_builder_get_object(builder, "closeButton"));
   	g_signal_connect(G_OBJECT(accountCloseButton), "clicked", G_CALLBACK(hideGtkWidget), mainWindow);
   	
   	GtkWidget	*addButton;
   	addButton = GTK_WIDGET(gtk_builder_get_object(builder, "addSystemButton"));
   	g_signal_connect(G_OBJECT(addButton), "clicked", G_CALLBACK(prepareSystemsAddItem), mainWindow);   	
   	
	/* Setup the accounts tree */
	accounts->accountContainer = GTK_WIDGET(gtk_builder_get_object(builder, "accountScroll"));
	accounts->accountTree = gtk_tree_view_new();
	setupSystemTree(accounts);
	gtk_container_add(GTK_CONTAINER(accounts->accountContainer), accounts->accountTree);

	/* Setup the account tree to be multi selection */
	accounts->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (accounts->accountTree));
	gtk_tree_selection_set_mode(accounts->selection, GTK_SELECTION_MULTIPLE);	

	/* Setup the search entry & button */
	accounts->searchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "searchEntry"));
	g_signal_connect(G_OBJECT(accounts->searchEntry), "activate", G_CALLBACK(getSystems), accounts);
	
	GtkWidget *searchButton = GTK_WIDGET(gtk_builder_get_object(builder, "searchButton"));
	g_signal_connect(G_OBJECT(searchButton), "clicked", G_CALLBACK(getSystems), accounts);

	/* Setup keypress signals on the tree */
	//g_signal_connect(accounts->accountTree, "button-press-event", G_CALLBACK(treeButtonPress), accounts);
	//g_signal_connect(accounts->accountTree, "key-press-event", G_CALLBACK(treeKeyPress), accounts);

	/* Load the inventory tree with all data from the database */
	//getAccounts(NULL, accounts);
	g_signal_connect(accounts->mainWindow, "show", G_CALLBACK(getSystems), accounts);
}

static void closeWindow(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(window);
}

static void freeWindow(GtkWidget *widget, intrackAccounts *accounts) {
	if(widget)
		gtk_widget_destroy(widget);
	
	freeMemory(accounts);
}

/* Free up memory after a new account has been created. */
static void freeMemory(intrackAccounts *accounts) {
	//g_free(accounts->selectedAccount);
	g_slice_free(accountsCreate, accounts->newAccount);
}

/* Opens a widget popup to prompt user to add a item. -> systems_add.c */
static void prepareSystemsAddItem(GtkWidget *widget, GtkWidget *mainWindow) {
	initalizeSystemAdd(mainWindow); // -> partSales_add.c
}

/* Feed query commands to the database */
/* Good for sending updating, creation and deletion commands. Do not use for querying results */
static int databaseQuery(gchar *query, gchar *database, GtkWidget *window) {
	int query_state;	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	query_state = mysql_select_db(connection, database);	
	query_state=mysql_query(connection, query);
	
	// Failed to update the data
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, window);
		g_free(errorMessage);
				
		mysql_close(connection);
		return 1;
	}		
	
	return 0;
}

/* Prepare to pull the system data out of the database */
static void getSystems(GtkWidget *widget, intrackAccounts *accounts) {
	GtkListStore *store;

	store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	if(strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) > 2) {
		gchar *searchString;	
		
		searchString = g_strdup(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry)));
		pullSystems(store, accounts, searchString);

		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(accounts->searchEntry), "");
	}
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, accounts->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(accounts->searchEntry), "");
	}
	else {
		pullSystems(store, accounts, NULL);
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(accounts->accountTree), GTK_TREE_MODEL(store));
	g_object_unref(store);	
}

static int pullSystems(GtkListStore *store, intrackAccounts *accounts, gchar *searchString) {
	GtkTreeIter 	iter;
	gchar *query_string;
	int query_state;
	int i;
	int num_fields;		
	
	/* Open MYSQL connection to the database */
	if(connectToServer() == 1) {
		return 1;
	}
	
    // Select the database.
    query_string = g_strdup(SYSTEMS_DATABASE);	
	query_state = mysql_select_db(connection, query_string);	
	g_free(query_string);
	
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}	
	
	/* Search for the user entered query */
	if(searchString != NULL)
		query_string = g_strconcat("SELECT id, catNum, serial1, serial2, serial3, dateIn, dateOut, soldTo, invoiceNum, ownedBy, notes FROM `", SYSTEMS_TABLES, "` WHERE catNum LIKE '%", searchString, "%' OR serial1 LIKE '%", searchString, "%' OR serial2 LIKE '%", searchString, "%' OR serial3 LIKE '%", searchString, "%' OR soldTo LIKE '%", searchString, "%' OR dateOut LIKE '%", searchString, "%' OR ownedBy LIKE '%", searchString, "%' OR notes LIKE '%", searchString,"%' OR invoiceNum LIKE '%", searchString, "%'", NULL);
	else	
		query_string = g_strconcat("SELECT id, catNum, serial1, serial2, serial3, dateIn, dateOut, soldTo, invoiceNum, ownedBy, notes FROM `", SYSTEMS_TABLES, "`", NULL);
		
	query_state = mysql_query(connection, query_string);
	g_free(query_string);
	
	// Failed to connect and query database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		
		gchar *errorMessage = g_strdup_printf("%s", mysql_error(connection));
		printMessage(errorMessage, accounts->mainWindow);
		g_free(errorMessage);
		
		mysql_close(connection);
		
		return 1;
	}
	
	result = mysql_store_result(connection);
	num_fields = mysql_num_fields(result);	
	
	while ((row = mysql_fetch_row(result))) {
		gchar *systemID, *catNum, *serial1, *serial2, *serial3, *dateIn, *dateOut, *soldTo, *invoiceNum, *ownedBy, *notes;

		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				systemID = g_strdup(row[i]);
				
			if(i == 1)
				catNum = g_strdup(row[i]);
			
			if(i == 2)
				serial1 = g_strdup(row[i]);
			
			if(i == 3)
				serial2 = g_strdup(row[i]);
				
			if(i == 4)
				serial3 = g_strdup(row[i]);

			if(i == 5)
				dateIn = g_strdup(row[i]);
				
			if(i == 6)
				dateOut = g_strdup(row[i]);
				
			if(i == 7)
				soldTo = g_strdup(row[i]);
			
			if(i == 8)
				invoiceNum = g_strdup(row[i]);
				
			if(i == 9)
				ownedBy = g_strdup(row[i]);
				
			if(i == 10)
				notes = g_strdup(row[i]);								
		}

		/* Store the results into the tree now */
		gtk_list_store_append (store, &iter);
		gtk_list_store_set(store, &iter, SYSTEM_ID, systemID, CATNUM, catNum, SERIAL1, serial1, SERIAL2, serial2, SERIAL3, serial3, DATEIN, dateIn, DATEOUT, dateOut, SOLDTO, soldTo, OWNEDBY, ownedBy, NOTES, notes, INVOICENUM, invoiceNum, -1);

		g_free(systemID), g_free(catNum), g_free(serial1), g_free(serial2), g_free(serial3), g_free(dateIn), g_free(dateOut), g_free(soldTo), g_free(invoiceNum), g_free(notes), g_free(ownedBy);
	}

	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}

/* Updates a item in the inventory database. */
static int databaseEditItem(gchar *whereID, gchar *newData, gchar *column) {
	
	gchar *query_string;
	int query_state;	

	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the systems database.
	query_string = g_strconcat(SYSTEMS_DATABASE, NULL);
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
	query_string = g_strconcat("UPDATE `", SYSTEMS_TABLES, "`", " SET ", column, "='", newData, "' WHERE id='", whereID, "'", NULL);

	query_state=mysql_query(connection, query_string);
	g_free(query_string);	
	
	/* Failed to update the data */
	if (query_state !=0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_UPDATING_INVENTORY, NULL);
				
		mysql_close(connection);

		return 1;
	}
	
	mysql_close(connection);			
	
	return 0;	
}

static int cellClickedCat(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

    if(strlen(newText) > 0) {
		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "catNum");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, CATNUM, newText, -1);
	}
	else if(strlen(newText) < 1) {
		printMessage(ERROR_LENGTH, accounts->mainWindow);
	}

    g_free(whereData);
	
	return 0;
}

static int cellClickedSerial1(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "serial1");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, SERIAL1, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedSerial2(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "serial2");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, SERIAL2, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedSerial3(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "serial3");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, SERIAL3, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedDateIn(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "dateIn");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, DATEIN, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedDateOut(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "dateOut");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, DATEOUT, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedSoldTo(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "soldTo");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, SOLDTO, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedOwnedBy(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "ownedBy");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, OWNEDBY, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedNotes(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "notes");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, NOTES, newText, -1);

    g_free(whereData);
	
	return 0;
}

static int cellClickedInvoiceNum(GtkCellRendererText *render, gchar *path, gchar *newText, intrackAccounts *accounts) {
	
	/* Checks to see if input is valid */
	if(checkInput(newText))
		return 1;
		
    GtkTreeModel 	*model;
    GtkTreeIter 	iter;
	
	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter_from_string(model, &iter, path);
    gtk_tree_model_get(model, &iter, SYSTEM_ID, &whereData, -1);	

		/* Update the systems database / table */
		databaseEditItem(whereData, newText, "invoiceNum");

		/* Update the tree cell with the new data */
		gtk_list_store_set(GTK_LIST_STORE(model), &iter, INVOICENUM, newText, -1);

    g_free(whereData);
	
	return 0;
}

/* Create the tree view and columns */
static void setupSystemTree(intrackAccounts *accounts) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", SYSTEM_ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. ID purposes only.
	gtk_tree_view_column_set_sort_column_id (column, SYSTEM_ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Cat #", renderer, "text", CATNUM, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, CATNUM); 
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedCat), (gpointer) accounts);		
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Serial #", renderer, "text", SERIAL1, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SERIAL1);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedSerial1), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Serial #", renderer, "text", SERIAL2, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SERIAL2);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedSerial2), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Serial #", renderer, "text", SERIAL3, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SERIAL3);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedSerial3), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Date In", renderer, "text", DATEIN, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DATEIN);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDateIn), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Date Out", renderer, "text", DATEOUT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DATEOUT);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedDateOut), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Sold To", renderer, "text", SOLDTO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SOLDTO);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedSoldTo), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Owned By", renderer, "text", OWNEDBY, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, OWNEDBY);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedOwnedBy), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Invoice #", renderer, "text", INVOICENUM, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, INVOICENUM);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedInvoiceNum), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Notes", renderer, "text", NOTES, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, NOTES);
	g_object_set (renderer, "editable", TRUE, "editable-set", TRUE, NULL);
	g_signal_connect (G_OBJECT (renderer), "edited", G_CALLBACK (cellClickedNotes), (gpointer) accounts);			
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);				
	
}

/* Generates a random account number */
static void generateAccountNumber(GtkWidget *widget, accountsCreate *newAccount) {
/*
#ifdef UNIX
#define seedr(a) srand48(a)
#define randm()  lrand48()
#else
#define seedr(a) srand(a)
#define randm()  rand()
#endif
*/	
	const char *charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
	int len=10;
	int i;
	char * out;
	
	//seedr(time(NULL));
	srand(time(NULL));

	out = malloc(len);

	for (i = 0; i < len; i++)
		out[i] = charset[rand() % strlen(charset)]; // out[i] = charset[randm() % strlen(charset)];
	
	// Set the end of the array to terminate as NULL
	out[len] = 0;
	
	gtk_entry_set_text(GTK_ENTRY(newAccount->accountEntry), out);
}

static void destroyGTKWidget(GtkWidget *widget, gpointer parentWindow) {
	gtk_widget_destroy(GTK_WIDGET(parentWindow));
}

static void hideGtkWidget(GtkWidget *widget, gpointer data) {
	gtk_widget_hide_all(GTK_WIDGET(data));
}

static void destroyModuleWindow(GtkWidget *widget, GtkWidget *window) {
	gtk_widget_destroy(GTK_WIDGET(window));
}

static void freeModuleMemory(GtkWidget *widget, intrackAccounts *accounts) {
	if(widget) {
		destroyModuleWindow(NULL, accounts->mainWindow);
		g_free(accounts->selectedAccount);
		g_free(accounts);
	}
	else {
		g_free(accounts->selectedAccount);
		g_free(accounts);
	}
}

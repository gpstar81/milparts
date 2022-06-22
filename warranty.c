//      warranty.c
//      Copyright 2011 Michael Rajotte <michael@michaelrajotte.com>

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
#include "warranty.h"

/* Loads up the fees configuration */
void loadWarranty(GtkBuilder *builder, GtkWidget *mainWindow) {
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
   	
	/* Setup the accounts tree */
	accounts->accountContainer = GTK_WIDGET(gtk_builder_get_object(builder, "accountScroll"));
	accounts->accountTree = gtk_tree_view_new();
	setupWarrantyTree(accounts);
	gtk_container_add(GTK_CONTAINER(accounts->accountContainer), accounts->accountTree);

	/* Setup the account tree to be multi selection */
	accounts->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW (accounts->accountTree));
	gtk_tree_selection_set_mode(accounts->selection, GTK_SELECTION_MULTIPLE);	

	/* Setup the search entry & button */
	accounts->searchEntry = GTK_WIDGET(gtk_builder_get_object(builder, "searchEntry"));
	g_signal_connect(G_OBJECT(accounts->searchEntry), "activate", G_CALLBACK(getWarranties), accounts);
	
	GtkWidget *searchButton = GTK_WIDGET(gtk_builder_get_object(builder, "searchButton"));
	g_signal_connect(G_OBJECT(searchButton), "clicked", G_CALLBACK(getWarranties), accounts);
	
   	accounts->deleteButton = GTK_WIDGET(gtk_builder_get_object(builder, "removeButton"));
   	g_signal_connect(G_OBJECT(accounts->deleteButton), "clicked", G_CALLBACK(deleteItemWindow), accounts);
   	gtk_widget_set_sensitive(accounts->deleteButton, FALSE);	

	/* Setup keypress signals on the tree */
	g_signal_connect(accounts->accountTree, "button-press-event", G_CALLBACK(treeButtonPress), accounts);
	g_signal_connect(accounts->accountTree, "key-press-event", G_CALLBACK(treeKeyPress), accounts);

	/* Load the inventory tree with all data from the database */
	//getAccounts(NULL, accounts);
	g_signal_connect(accounts->mainWindow, "show", G_CALLBACK(getWarranties), accounts);
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

/* A widget window to ask for confirmation upon deletion */
static void deleteItemWindow(GtkWidget *widget, intrackAccounts *accounts) {
	GtkWidget *dialog;
	gint widgetDialog;
	
	dialog = gtk_message_dialog_new(GTK_WINDOW(accounts->mainWindow), 
			GTK_DIALOG_DESTROY_WITH_PARENT, 
			GTK_MESSAGE_WARNING, 
			GTK_BUTTONS_YES_NO, 
			"Remove selected warranty items?\n\nWARNING: This is permanent and non reversible!");
			
	gtk_window_set_title(GTK_WINDOW(dialog), "Remove items?");
	widgetDialog = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (widgetDialog) {
		case GTK_RESPONSE_YES:
			prepareItemRemoval(NULL, accounts);
			break;
		
		case GTK_RESPONSE_NO:
			break;
	}
}

/* Remove a item from the inventory database */
static void beginItemRemoval(GtkTreeRowReference *ref, intrackAccounts *accounts) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *whereData;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &whereData, -1); /* id column */
	
	/* Update the database */
	//databaseEditItem(inventory->mysqlDatabase, inventory->inventoryTable, partNo, newStockChar, "stock");
		
	/* Update the database */
	databaseRemoveItem(whereData);
		
	g_free(whereData);
}

/* Prepares the remove process of a item from the inventory */
static void prepareItemRemoval(GtkWidget *widget, intrackAccounts *accounts) {

	GtkTreeSelection *selection;
	GtkTreeRowReference *ref;
	GtkTreeModel *model;
	GList *rows, *ptr, *references = NULL;

	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(accounts->accountTree));
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
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
	g_list_foreach(references, (GFunc) beginItemRemoval, accounts);
	
	/* Free the tree paths, tree row references and lists */
	g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
	g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
	g_list_free(references);
	g_list_free(rows);
	
	getWarranties(NULL, accounts);
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

/* Prepare to pull the account data out of the database */
static void getWarranties(GtkWidget *widget, intrackAccounts *accounts) {
	GtkListStore *store;

	store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	
	if(strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) > 2) {
		gchar *searchString;	
		
		searchString = g_strdup(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry)));
		pullWarranty(store, accounts, searchString);

		g_free(searchString);
		gtk_entry_set_text(GTK_ENTRY(accounts->searchEntry), "");
	}
	else if(strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) > 0 && strlen(gtk_entry_get_text(GTK_ENTRY(accounts->searchEntry))) < 3) {
		printMessage(ERROR_SEARCH_TERMS, accounts->mainWindow);
		gtk_entry_set_text(GTK_ENTRY(accounts->searchEntry), "");
	}
	else {
		pullWarranty(store, accounts, NULL);
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(accounts->accountTree), GTK_TREE_MODEL(store));
	g_object_unref(store);	
}

static int pullWarranty(GtkListStore *store, intrackAccounts *accounts, gchar *searchString) {
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
    query_string = g_strdup(WARRANTY_DATABASE);	
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
		query_string = g_strconcat("SELECT id, dateEntered, name, email, systemCat, purchasedFrom, date, headCat, headNumber, tripodCat, tripodNumber FROM `", WARRANTY_TABLES, "` WHERE name LIKE '%", searchString, "%' OR email LIKE '%", searchString, "%' OR systemCat LIKE '%", searchString, "%' OR purchasedFrom LIKE '%", searchString, "%' OR headCat LIKE '%", searchString, "%' OR headNumber LIKE '%", searchString, "%' OR tripodCat LIKE '%", searchString, "%' OR tripodNumber LIKE '%", searchString, "%'", NULL);
	else	
		query_string = g_strconcat("SELECT id, dateEntered, name, email, systemCat, purchasedFrom, date, headCat, headNumber, tripodCat, tripodNumber FROM `", WARRANTY_TABLES, "`", NULL);
		
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
		gchar *id, *dateEntered, *name, *email, *systemCat, *purchasedFrom, *date, *headCat, *headNumber, *tripodCat, *tripodNumber;

		for(i = 0; i < num_fields; i++) {
			if(i == 0)
				id = g_strdup(row[i]);
			
			if(i == 1)
				dateEntered = g_strdup(row[i]);
			
			if(i == 2)
				name = g_strdup(row[i]);
			
			if(i == 3)
				email = g_strdup(row[i]);
				
			if(i == 4)
				systemCat = g_strdup(row[i]);

			if(i == 5)
				purchasedFrom = g_strdup(row[i]);
				
			if(i == 6)
				date = g_strdup(row[i]);
				
			if(i == 7)
				headCat = g_strdup(row[i]);
			
			if(i == 8)
				headNumber = g_strdup(row[i]);
				
			if(i == 9)
				tripodCat = g_strdup(row[i]);
				
			if(i == 10)
				tripodNumber = g_strdup(row[i]);
		}

		/* Store the results into the tree now */
		gtk_list_store_append (store, &iter);
			
		gtk_list_store_set(store, &iter, ID, id, DATE_REG, dateEntered, NAME, name, EMAIL, email, PURCHASED_FROM, purchasedFrom, DATE_PURCHASED, date, SYSTEM_CAT, systemCat, HEAD_CAT, headCat, HEAD_NO, headNumber, TRIPOD_CAT, tripodCat, TRIPOD_NO, tripodNumber, -1);

		g_free(id);
		g_free(dateEntered), g_free(name), g_free(email), g_free(systemCat), g_free(purchasedFrom);
		g_free(date), g_free(headCat), g_free(headNumber), g_free(tripodCat), g_free(tripodNumber);
	}

	mysql_free_result(result);
	mysql_close(connection);
	
	return 0;
}

/* Removes a item from the warranty database */
static int databaseRemoveItem(gchar *id) {

	gchar *query_string;
	int query_state;	
	
	if(connectToServer() == 1) {
		return 1;
	}
	
	// Select the database.
	query_string = g_strdup(WARRANTY_DATABASE);
	query_state = mysql_select_db(connection, query_string);
	g_free(query_string);
		
	// Failed to connect and select database.	
	if (query_state != 0) {
		printf(mysql_error(connection), "%d\n");
		printMessage(ERROR_DATABASE, NULL);
		
		mysql_close(connection);
		
		return 1;
	}
	
	query_string = g_strconcat("DELETE FROM `", WARRANTY_TABLES, "`", " WHERE id='", id, "'", NULL);

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

/* Create the tree view and columns */
static void setupWarrantyTree(intrackAccounts *accounts) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", ID, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_visible(column, FALSE); // Hide this column. ID purposes only.
	gtk_tree_view_column_set_sort_column_id (column, ID); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	
		
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Date Registered", renderer, "text", DATE_REG, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DATE_REG); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);	

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Name", renderer, "text", NAME, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, NAME); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Email", renderer, "text", EMAIL, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, EMAIL); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Purchased From", renderer, "text", PURCHASED_FROM, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, PURCHASED_FROM); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Date Purchased", renderer, "text", DATE_PURCHASED, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, DATE_PURCHASED); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("System Cat", renderer, "text", SYSTEM_CAT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, SYSTEM_CAT); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Head Cat", renderer, "text", HEAD_CAT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, HEAD_CAT); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);			
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Head #", renderer, "text", HEAD_NO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, HEAD_NO); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);		
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Tripod Cat", renderer, "text", TRIPOD_CAT, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, TRIPOD_CAT); 
	gtk_tree_view_append_column (GTK_TREE_VIEW (accounts->accountTree), column);
	
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes("Tripod #", renderer, "text", TRIPOD_NO, NULL);
	gtk_tree_view_column_set_resizable(column, TRUE);
	gtk_tree_view_column_set_reorderable(column, TRUE);
	gtk_tree_view_column_set_sort_column_id (column, TRIPOD_NO); 
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

/* Mouse click event on the tree */
static gboolean treeButtonPress(GtkWidget *widget, GdkEventButton *ev, intrackAccounts *accounts) {

    GtkTreePath *path;
    
	/* if there's no path where the click occurred... */
    if(!gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(accounts->accountTree), ev->x, ev->y, &path, NULL, NULL, NULL)) {
		gtk_widget_set_sensitive(accounts->deleteButton, FALSE);
        return FALSE;
	}

	/* Set the sensitivity on the viewItem Button */
	g_timeout_add(1, selectionTimer, (gpointer) accounts); 	

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
	
	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
    gtk_tree_model_get_iter(model, &iter, path);
    gtk_tree_model_get(model, &iter, ID, &rowBarcode, -1); /* 1 is the id column */
    
    g_free(accounts->selectedAccount);
    accounts->selectedAccount = g_strconcat(rowBarcode, NULL);
   	gtk_widget_set_sensitive(accounts->deleteButton, TRUE);
    g_free(rowBarcode);
    
    /* free our path */
    gtk_tree_path_free(path);  

    return FALSE;	
}

static gboolean selectionTimer(gpointer data) {
	
	intrackAccounts *accounts = (intrackAccounts *)data;

	/* If the user selects multiple rows in the basket tree, then turn off the itemView button */
	if(gtk_tree_selection_count_selected_rows(accounts->selection) > 0) {
		gtk_widget_set_sensitive(accounts->deleteButton, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);	
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
	}
	else {
		gtk_widget_set_sensitive(accounts->deleteButton, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, TRUE);		
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->editItemButton, TRUE);
	}		
	
	return FALSE;
}

/* Keyboard key press event on the inventory tree */
static gboolean treeKeyPress(GtkWidget *widget, GdkEventKey *ev, intrackAccounts *accounts) {
	
    switch(ev->keyval)
    {       
        case GDK_Up:
				/* Add a 1 millisecond delay before pulling the selection from the tree to get accurate selection. */
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;

        case GDK_KP_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;

        case GDK_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;

        case GDK_KP_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_KP_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;              
            
        case GDK_KP_End:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;     
            
        case GDK_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  

        case GDK_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_KP_Page_Up:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_KP_Page_Down:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;      
            
        case GDK_Home:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;  
            
        case GDK_End:
				g_timeout_add (1, keyPressSelection, (gpointer) accounts); 		
            break;
            		
        case GDK_Delete:
				deleteItemWindow(NULL, accounts);
			break;
			
		case GDK_KP_Delete:
				deleteItemWindow(NULL, accounts);
			break;                      
    }
	
    return FALSE;	
}

static gboolean keyPressSelection(gpointer data) {

	intrackAccounts *accounts = (intrackAccounts *)data;

	if(gtk_tree_selection_count_selected_rows(accounts->selection) == 0) {
		gtk_widget_set_sensitive(accounts->deleteButton, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
	}
	else if(gtk_tree_selection_count_selected_rows(accounts->selection) > 1) {
		gtk_widget_set_sensitive(accounts->deleteButton, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, FALSE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, FALSE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->editItemButton, FALSE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
	}
	else {
		gtk_widget_set_sensitive(accounts->deleteButton, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuView, TRUE);
		//gtk_widget_set_sensitive(inventory->invenMenuEdit, TRUE);
		//gtk_widget_set_sensitive(inventory->invenViewItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->removeItemButton, TRUE);
		//gtk_widget_set_sensitive(inventory->editItemButton, TRUE);
		
		GtkTreeSelection *selection;
		GtkTreeRowReference *ref;
		GtkTreeModel *model;
		GList *rows, *ptr, *references = NULL;

		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(accounts->accountTree));
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
		rows = gtk_tree_selection_get_selected_rows(selection, &model);
	
		ptr = rows;
		// Create tree row references to all of the selected rows
		while(ptr != NULL) {
			ref = gtk_tree_row_reference_new(model, (GtkTreePath*) ptr->data);
			references = g_list_prepend(references, gtk_tree_row_reference_copy(ref));
			gtk_tree_row_reference_free(ref);
			ptr = ptr->next;
		}

		g_list_foreach(references, (GFunc) keyPressGetRow, accounts);

		// Free the tree paths, tree row references and lists
		g_list_foreach(references, (GFunc) gtk_tree_row_reference_free, NULL);
		g_list_foreach(rows, (GFunc) gtk_tree_path_free, NULL);
		g_list_free(references);
		g_list_free(rows);
		
	}

	return FALSE;	
}

/* Get the selected row from a key press (part #2) */
static void keyPressGetRow(GtkTreeRowReference *ref, intrackAccounts *accounts) {
	GtkTreeIter parent, iter;
	GtkTreePath *path;
	GtkTreeModel *model;

	gchar *rowBarcode;

	model = gtk_tree_view_get_model(GTK_TREE_VIEW(accounts->accountTree));
	path = gtk_tree_row_reference_get_path (ref);
	gtk_tree_model_get_iter (model, &iter, path);

    gtk_tree_model_get(model, &iter, ID, &rowBarcode, -1); /* 1 is the barcode column */
    
    g_free(accounts->selectedAccount);
    accounts->selectedAccount = g_strconcat(rowBarcode, NULL);
    g_free(rowBarcode);
}
